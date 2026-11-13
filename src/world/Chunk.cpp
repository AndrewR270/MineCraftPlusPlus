#include "Chunk.h"
#include "BlockRegistry.h"
#include "FastNoiseLite.h"
#include <cmath>
#include <cstdlib>

// ============================================================================
// WORLD & CAVE GENERATION TUNING PARAMETERS
// You can tweak these variables to adjust terrain shape and cave morphology.
// ============================================================================
static float WORLD_TERRAIN_FREQ       = 0.015f;  // Controls width of surface hills (lower = wider, rolling hills).
static float CAVE_FREQ_XZ             = 0.035f;  // Controls horizontal scale and winding frequency of cave tunnels.
static float CAVE_FREQ_Y              = 0.075f;  // Controls vertical scale of caves. Lower this independently of XZ to flatten caves horizontally.
static float CAVE_DENSITY_THRESHOLD   = 0.15f;  // Controls tunnel thickness (larger = massive caverns, smaller = tight, narrow tunnels).
static int   SURFACE_BUFFER           = 20;       // Number of solid stone blocks maintained beneath the dirt/surface to dome cave ceilings naturally.
// ============================================================================

Chunk::Chunk(int chunkX, int chunkZ, unsigned int seed) {
    FastNoiseLite terrainNoise;
    terrainNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    terrainNoise.SetFrequency(WORLD_TERRAIN_FREQ);
    terrainNoise.SetSeed(static_cast<int>(seed));

    FastNoiseLite caveNoise;
    caveNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    caveNoise.SetFrequency(1.0f);
    caveNoise.SetSeed(static_cast<int>(seed + 1));

    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int z = 0; z < CHUNK_DEPTH; z++) {
            float worldX = static_cast<float>(chunkX * CHUNK_WIDTH + x);
            float worldZ = static_cast<float>(chunkZ * CHUNK_DEPTH + z);

            float noiseVal = terrainNoise.GetNoise(worldX, worldZ);
            int surfaceHeight = static_cast<int>(64.0f + noiseVal * 12.0f);

            for (int y = 0; y < CHUNK_HEIGHT; y++) {
                if (y == 0) {
                    blocks[x][y][z] = BlockID::BEDROCK;
                }
                else if (y < 5) {
                    bool isBedrock = (rand() % (y + 1) == 0) || (y == 1);
                    blocks[x][y][z] = isBedrock ? BlockID::BEDROCK : BlockID::STONE;
                }
                else if (y == surfaceHeight) {
                    blocks[x][y][z] = BlockID::GRASS;
                }
                else if (y > surfaceHeight - 4 && y < surfaceHeight) {
                    blocks[x][y][z] = BlockID::DIRT;
                }
                else if (y >= surfaceHeight) {
                    blocks[x][y][z] = BlockID::AIR;
                }
                else {
                    // Check distance from the local surface height
                    int depthBelowSurface = surfaceHeight - y;

                    if (depthBelowSurface > SURFACE_BUFFER) {
                        // Deep underground: full unconstrained cave generation
                        float sampleX = worldX * CAVE_FREQ_XZ;
                        float sampleY = static_cast<float>(y) * CAVE_FREQ_Y;
                        float sampleZ = worldZ * CAVE_FREQ_XZ;

                        float cVal = caveNoise.GetNoise(sampleX, sampleY, sampleZ);
                        bool isCave = std::abs(cVal) < CAVE_DENSITY_THRESHOLD;

                        blocks[x][y][z] = isCave ? BlockID::AIR : BlockID::STONE;
                    } 
                    else {
                        // Surface transition buffer: smoothly taper cave size to 0 as we get close to dirt,
                        // creating natural, domed stone ceilings instead of flat clipped shelves.
                        float sampleX = worldX * CAVE_FREQ_XZ;
                        float sampleY = static_cast<float>(y) * CAVE_FREQ_Y;
                        float sampleZ = worldZ * CAVE_FREQ_XZ;

                        float cVal = caveNoise.GetNoise(sampleX, sampleY, sampleZ);
                        
                        // Scale threshold down based on proximity to the surface buffer boundary
                        float taperFactor = static_cast<float>(depthBelowSurface) / static_cast<float>(SURFACE_BUFFER);
                        float taperedThreshold = CAVE_DENSITY_THRESHOLD * taperFactor;

                        bool isCave = std::abs(cVal) < taperedThreshold;
                        blocks[x][y][z] = isCave ? BlockID::AIR : BlockID::STONE;
                    }
                }
            }
        }
    }
}

BlockID Chunk::GetBlock(int x, int y, int z) const
{
    if (x < 0 || x >= CHUNK_WIDTH || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_DEPTH)
        return BlockID::AIR;
    return blocks[x][y][z];
}

void Chunk::SetBlock(int x, int y, int z, BlockID id)
{
    if (x >= 0 && x < CHUNK_WIDTH && y >= 0 && y < CHUNK_HEIGHT && z >= 0 && z < CHUNK_DEPTH)
    {
        blocks[x][y][z] = id;
    }
}

std::vector<float> Chunk::GenerateMesh() const
{
    std::vector<float> vertices;

    auto isSolid = [](BlockID id)
    {
        return id != BlockID::AIR;
    };

    for (int x = 0; x < CHUNK_WIDTH; x++)
    {
        for (int y = 0; y < CHUNK_HEIGHT; y++)
        {
            for (int z = 0; z < CHUNK_DEPTH; z++)
            {
                BlockID currentId = blocks[x][y][z];
                if (currentId == BlockID::AIR)
                    continue;

                const BlockData &data = BlockRegistry::GetInstance().GetBlock(currentId);
                float tintFlag = data.hasGrassTint ? 1.0f : 0.0f;
                UVCoord t = data.top;
                UVCoord b = data.bottom;
                UVCoord s = data.side;

                // 1. Right Face (+X)
                if (!isSolid(GetBlock(x + 1, y, z)))
                {
                    float faceVerts[] = {
                        x + 0.5f, y + 0.5f, z + 0.5f, 0.6f, s.uMax, s.vMax, 0.0f,
                        x + 0.5f, y - 0.5f, z + 0.5f, 0.6f, s.uMax, s.vMin, 0.0f,
                        x + 0.5f, y - 0.5f, z - 0.5f, 0.6f, s.uMin, s.vMin, 0.0f,
                        x + 0.5f, y - 0.5f, z - 0.5f, 0.6f, s.uMin, s.vMin, 0.0f,
                        x + 0.5f, y + 0.5f, z - 0.5f, 0.6f, s.uMin, s.vMax, 0.0f,
                        x + 0.5f, y + 0.5f, z + 0.5f, 0.6f, s.uMax, s.vMax, 0.0f};
                    vertices.insert(vertices.end(), std::begin(faceVerts), std::end(faceVerts));
                }

                // 2. Left Face (-X)
                if (!isSolid(GetBlock(x - 1, y, z)))
                {
                    float faceVerts[] = {
                        x - 0.5f, y + 0.5f, z - 0.5f, 0.6f, s.uMax, s.vMax, 0.0f,
                        x - 0.5f, y - 0.5f, z - 0.5f, 0.6f, s.uMax, s.vMin, 0.0f,
                        x - 0.5f, y - 0.5f, z + 0.5f, 0.6f, s.uMin, s.vMin, 0.0f,
                        x - 0.5f, y - 0.5f, z + 0.5f, 0.6f, s.uMin, s.vMin, 0.0f,
                        x - 0.5f, y + 0.5f, z + 0.5f, 0.6f, s.uMin, s.vMax, 0.0f,
                        x - 0.5f, y + 0.5f, z - 0.5f, 0.6f, s.uMax, s.vMax, 0.0f};
                    vertices.insert(vertices.end(), std::begin(faceVerts), std::end(faceVerts));
                }

                // 3. Top Face (+Y)
                if (!isSolid(GetBlock(x, y + 1, z)))
                {
                    float faceVerts[] = {
                        x + 0.5f, y + 0.5f, z + 0.5f, 1.0f, t.uMax, t.vMax, tintFlag,
                        x + 0.5f, y + 0.5f, z - 0.5f, 1.0f, t.uMax, t.vMin, tintFlag,
                        x - 0.5f, y + 0.5f, z - 0.5f, 1.0f, t.uMin, t.vMin, tintFlag,
                        x - 0.5f, y + 0.5f, z - 0.5f, 1.0f, t.uMin, t.vMin, tintFlag,
                        x - 0.5f, y + 0.5f, z + 0.5f, 1.0f, t.uMin, t.vMax, tintFlag,
                        x + 0.5f, y + 0.5f, z + 0.5f, 1.0f, t.uMax, t.vMax, tintFlag};
                    vertices.insert(vertices.end(), std::begin(faceVerts), std::end(faceVerts));
                }

                // 4. Bottom Face (-Y)
                if (!isSolid(GetBlock(x, y - 1, z)))
                {
                    float faceVerts[] = {
                        x + 0.5f, y - 0.5f, z + 0.5f, 0.4f, b.uMax, b.vMax, 0.0f,
                        x + 0.5f, y - 0.5f, z - 0.5f, 0.4f, b.uMax, b.vMin, 0.0f,
                        x - 0.5f, y - 0.5f, z - 0.5f, 0.4f, b.uMin, b.vMin, 0.0f,
                        x - 0.5f, y - 0.5f, z - 0.5f, 0.4f, b.uMin, b.vMin, 0.0f,
                        x - 0.5f, y - 0.5f, z + 0.5f, 0.4f, b.uMin, b.vMax, 0.0f,
                        x + 0.5f, y - 0.5f, z + 0.5f, 0.4f, b.uMax, b.vMax, 0.0f};
                    vertices.insert(vertices.end(), std::begin(faceVerts), std::end(faceVerts));
                }

                // 5. Front Face (+Z)
                if (!isSolid(GetBlock(x, y, z + 1)))
                {
                    float faceVerts[] = {
                        x - 0.5f, y + 0.5f, z + 0.5f, 0.8f, s.uMin, s.vMax, 0.0f,
                        x - 0.5f, y - 0.5f, z + 0.5f, 0.8f, s.uMin, s.vMin, 0.0f,
                        x + 0.5f, y - 0.5f, z + 0.5f, 0.8f, s.uMax, s.vMin, 0.0f,
                        x + 0.5f, y - 0.5f, z + 0.5f, 0.8f, s.uMax, s.vMin, 0.0f,
                        x + 0.5f, y + 0.5f, z + 0.5f, 0.8f, s.uMax, s.vMax, 0.0f,
                        x - 0.5f, y + 0.5f, z + 0.5f, 0.8f, s.uMin, s.vMax, 0.0f};
                    vertices.insert(vertices.end(), std::begin(faceVerts), std::end(faceVerts));
                }

                // 6. Back Face (-Z)
                if (!isSolid(GetBlock(x, y, z - 1)))
                {
                    float faceVerts[] = {
                        x + 0.5f, y + 0.5f, z - 0.5f, 0.8f, s.uMax, s.vMax, 0.0f,
                        x + 0.5f, y - 0.5f, z - 0.5f, 0.8f, s.uMax, s.vMin, 0.0f,
                        x - 0.5f, y - 0.5f, z - 0.5f, 0.8f, s.uMin, s.vMin, 0.0f,
                        x - 0.5f, y - 0.5f, z - 0.5f, 0.8f, s.uMin, s.vMin, 0.0f,
                        x - 0.5f, y + 0.5f, z - 0.5f, 0.8f, s.uMin, s.vMax, 0.0f,
                        x + 0.5f, y + 0.5f, z - 0.5f, 0.8f, s.uMax, s.vMax, 0.0f};
                    vertices.insert(vertices.end(), std::begin(faceVerts), std::end(faceVerts));
                }
            }
        }
    }

    return vertices;
}