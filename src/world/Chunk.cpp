#include "Chunk.h"
#include "BlockRegistry.h"
#include "FastNoiseLite.h"
#include <cmath>
#include <cstdlib>

// ============================================================================
// WORLD & CAVE GENERATION TUNING PARAMETERS
// ============================================================================
static float WORLD_TERRAIN_FREQ       = 0.015f;  // Controls width of surface hills
static float CAVE_FREQ_XZ             = 0.015f;  // Controls horizontal scale and winding frequency of caves
static float CAVE_FREQ_Y              = 0.035f;  // Controls vertical scale of caves
static float CAVE_DENSITY_THRESHOLD   = 0.045f;  // Controls tunnel thickness / radius squared
static int   SURFACE_BUFFER           = 20;      // Solid stone buffer beneath dirt to dome cave ceilings
// ============================================================================

// ============================================================================
// AUTHENTIC MINECRAFT JAVA 1.0 ORE GENERATION PARAMETERS
// ============================================================================
static int COAL_VEINS     = 20; static int COAL_SIZE     = 17; static int COAL_MIN_Y     = 5; static int COAL_MAX_Y     = 128;
static int IRON_VEINS     = 20; static int IRON_SIZE     = 8;  static int IRON_MIN_Y     = 5; static int IRON_MAX_Y     = 64;
static int GOLD_VEINS     = 2;  static int GOLD_SIZE     = 8;  static int GOLD_MIN_Y     = 5; static int GOLD_MAX_Y     = 32;
static int LAPIS_VEINS    = 1;  static int LAPIS_SIZE    = 6;  static int LAPIS_MIN_Y    = 5; static int LAPIS_MAX_Y    = 32;
static int REDSTONE_VEINS = 8;  static int REDSTONE_SIZE = 8;  static int REDSTONE_MIN_Y = 5; static int REDSTONE_MAX_Y = 16;
static int DIAMOND_VEINS  = 1;  static int DIAMOND_SIZE  = 7;  static int DIAMOND_MIN_Y  = 5; static int DIAMOND_MAX_Y  = 16;
// ============================================================================

// Deterministic Pseudo-Random Number Generator for Chunks
struct ChunkRNG {
    unsigned int state;
    ChunkRNG(int cx, int cz, int salt, unsigned int worldSeed) {
        state = worldSeed ^ (cx * 73856093) ^ (cz * 19349663) ^ (salt * 83492791);
    }
    int NextInt(int max) {
        state = state * 1103515245 + 12345;
        return static_cast<int>((state >> 16) % max);
    }
    float NextFloat() {
        state = state * 1103515245 + 12345;
        return static_cast<float>((state >> 16) & 0xFFFF) / 65535.0f;
    }
};

Chunk::Chunk(int chunkX, int chunkZ, unsigned int seed) {
    FastNoiseLite terrainNoise;
    terrainNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    terrainNoise.SetFrequency(WORLD_TERRAIN_FREQ);
    terrainNoise.SetSeed(static_cast<int>(seed));

    // Dual noise fields for true cylindrical/tubular tunnel carving
    FastNoiseLite caveNoise1;
    caveNoise1.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    caveNoise1.SetFrequency(1.0f);
    caveNoise1.SetSeed(static_cast<int>(seed + 1));

    FastNoiseLite caveNoise2;
    caveNoise2.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    caveNoise2.SetFrequency(1.0f);
    caveNoise2.SetSeed(static_cast<int>(seed + 99));

    // 1. Pass 1: Generate Terrain, Bedrock, Soil, and Tubular Caves
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
                    int depthBelowSurface = surfaceHeight - y;
                    float sampleX = worldX * CAVE_FREQ_XZ;
                    float sampleY = static_cast<float>(y) * CAVE_FREQ_Y;
                    float sampleZ = worldZ * CAVE_FREQ_XZ;

                    // Dual-noise squared distance field for cylindrical tubes (barrels)
                    float n1 = caveNoise1.GetNoise(sampleX, sampleY, sampleZ);
                    float n2 = caveNoise2.GetNoise(sampleX + 43.5f, sampleY + 43.5f, sampleZ + 43.5f);
                    float tubeVal = (n1 * n1) + (n2 * n2);

                    float taperFactor = (depthBelowSurface > SURFACE_BUFFER) ? 1.0f : (static_cast<float>(depthBelowSurface) / static_cast<float>(SURFACE_BUFFER));
                    bool isCave = tubeVal < (CAVE_DENSITY_THRESHOLD * taperFactor);

                    blocks[x][y][z] = isCave ? BlockID::AIR : BlockID::STONE;
                }
            }
        }
    }

    // 2. Pass 2: Authentic Minecraft Vein Generator (WorldGenMinable)
    auto placeOreVein = [&](BlockID oreId, int veins, int size, int minY, int maxY, int salt) {
        ChunkRNG rng(chunkX, chunkZ, salt, seed);
        for (int i = 0; i < veins; i++) {
            float xCoord = static_cast<float>(rng.NextInt(CHUNK_WIDTH));
            float yCoord = static_cast<float>(minY + rng.NextInt(maxY - minY));
            float zCoord = static_cast<float>(rng.NextInt(CHUNK_DEPTH));

            for (int j = 0; j < size; j++) {
                float angle1 = rng.NextFloat() * 3.14159f * 2.0f;
                float yOffset = (rng.NextFloat() - 0.5f) * 2.0f;

                xCoord += sinf(angle1) * cosf(yOffset);
                yOffset += (rng.NextFloat() - 0.5f) * 0.5f;
                zCoord += cosf(angle1) * cosf(yOffset);

                int bx = static_cast<int>(roundf(xCoord));
                int by = static_cast<int>(roundf(yCoord));
                int bz = static_cast<int>(roundf(zCoord));

                if (bx >= 0 && bx < CHUNK_WIDTH && by >= minY && by < maxY && bz >= 0 && bz < CHUNK_DEPTH) {
                    if (blocks[bx][by][bz] == BlockID::STONE) {
                        blocks[bx][by][bz] = oreId;
                    }
                }
            }
        }
    };

    placeOreVein(BlockID::COAL_ORE,     COAL_VEINS,     COAL_SIZE,     COAL_MIN_Y,     COAL_MAX_Y,     101);
    placeOreVein(BlockID::IRON_ORE,     IRON_VEINS,     IRON_SIZE,     IRON_MIN_Y,     IRON_MAX_Y,     102);
    placeOreVein(BlockID::GOLD_ORE,     GOLD_VEINS,     GOLD_SIZE,     GOLD_MIN_Y,     GOLD_MAX_Y,     103);
    placeOreVein(BlockID::LAPIS_ORE,    LAPIS_VEINS,    LAPIS_SIZE,    LAPIS_MIN_Y,    LAPIS_MAX_Y,    104);
    placeOreVein(BlockID::REDSTONE_ORE, REDSTONE_VEINS, REDSTONE_SIZE, REDSTONE_MIN_Y, REDSTONE_MAX_Y, 105);
    placeOreVein(BlockID::DIAMOND_ORE,  DIAMOND_VEINS,  DIAMOND_SIZE,  DIAMOND_MIN_Y,  DIAMOND_MAX_Y,  106);
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