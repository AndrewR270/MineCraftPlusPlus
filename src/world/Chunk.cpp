#include "Chunk.h"
#include "World.h"
#include "BlockRegistry.h"
#include "FastNoiseLite.h"
#include <cmath>
#include <cstdlib>

// ============================================================================
// WORLD & CAVE GENERATION TUNING PARAMETERS
// ============================================================================
static float WORLD_TERRAIN_FREQ = 0.015f;     // Controls width of surface hills
static float CAVE_FREQ_XZ = 0.015f;           // Controls horizontal scale and winding frequency of caves
static float CAVE_FREQ_Y = 0.025f;            // Controls vertical scale of caves
static float CAVE_DENSITY_THRESHOLD = 0.0025f; // Controls tunnel thickness / radius squared
// ============================================================================

// Authentic deterministic PRNG structure for reproducible ore generation across chunks
struct ChunkRNG
{
    unsigned int state;
    ChunkRNG(int cx, int cz, int salt, unsigned int worldSeed)
    {
        state = worldSeed ^ (cx * 73856093) ^ (cz * 19349663) ^ (salt * 83492791);
    }
    int NextInt(int max)
    {
        state = state * 1103515245 + 12345;
        return static_cast<int>((state >> 16) % max);
    }
    float NextFloat()
    {
        state = state * 1103515245 + 12345;
        return static_cast<float>((state >> 16) & 0xFFFF) / 65535.0f;
    }
};

// Deterministic hash for surface variation
static int GetSurfaceNoise(int x, int z, unsigned int seed)
{
    unsigned int n = x * 73856093 + z * 19349663 + seed * 83492791;
    n = (n ^ (n >> 13)) * 1274126177;
    return static_cast<int>((n >> 16) % 3); 
}

// Chunk Constructor: Executes CPU-side procedural generation (Heightmaps, Caves, Ores)
Chunk::Chunk(int chunkX, int chunkZ, unsigned int seed)
    : m_chunkX(chunkX), m_chunkZ(chunkZ)
{
    FastNoiseLite terrainNoise;
    terrainNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    terrainNoise.SetFrequency(WORLD_TERRAIN_FREQ);
    terrainNoise.SetSeed(static_cast<int>(seed));

    FastNoiseLite caveNoise1;
    caveNoise1.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    caveNoise1.SetFrequency(1.0f);
    caveNoise1.SetSeed(static_cast<int>(seed + 1));

    FastNoiseLite caveNoise2;
    caveNoise2.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    caveNoise2.SetFrequency(1.0f);
    caveNoise2.SetSeed(static_cast<int>(seed + 99));

    // Pre-calculate surface heights with a 1-block border for slope/gradient checks
    int heightMap[CHUNK_WIDTH + 2][CHUNK_DEPTH + 2];
    for (int x = -1; x <= CHUNK_WIDTH; x++)
    {
        for (int z = -1; z <= CHUNK_DEPTH; z++)
        {
            float worldX = static_cast<float>(chunkX * CHUNK_WIDTH + x);
            float worldZ = static_cast<float>(chunkZ * CHUNK_DEPTH + z);
            float noiseVal = terrainNoise.GetNoise(worldX, worldZ);
            heightMap[x + 1][z + 1] = static_cast<int>(64.0f + noiseVal * 12.0f);
        }
    }

    bool isSolidBlock[CHUNK_WIDTH][CHUNK_HEIGHT][CHUNK_DEPTH] = {{{false}}};

    // Pass 1: Generate Terrain, Bedrock, and Caves
    for (int x = 0; x < CHUNK_WIDTH; x++)
    {
        for (int z = 0; z < CHUNK_DEPTH; z++)
        {
            float worldX = static_cast<float>(chunkX * CHUNK_WIDTH + x);
            float worldZ = static_cast<float>(chunkZ * CHUNK_DEPTH + z);

            int surfaceHeight = heightMap[x + 1][z + 1];

            for (int y = 0; y < CHUNK_HEIGHT; y++)
            {
                if (y == 0)
                {
                    blocks[x][y][z] = BlockID::BEDROCK;
                    isSolidBlock[x][y][z] = true;
                }
                else if (y < 5)
                {
                    bool isBedrock = (rand() % (y + 1) == 0) || (y == 1);
                    blocks[x][y][z] = isBedrock ? BlockID::BEDROCK : BlockID::STONE;
                    isSolidBlock[x][y][z] = true;
                }
                else if (y > surfaceHeight + 6)
                {
                    blocks[x][y][z] = BlockID::AIR;
                    isSolidBlock[x][y][z] = false;
                }
                else
                {
                    float sampleX = worldX * CAVE_FREQ_XZ;
                    float sampleY = static_cast<float>(y) * CAVE_FREQ_Y;
                    float sampleZ = worldZ * CAVE_FREQ_XZ;

                    float n1 = caveNoise1.GetNoise(sampleX, sampleY, sampleZ);
                    float n2 = caveNoise2.GetNoise(sampleX + 43.5f, sampleY + 43.5f, sampleZ + 43.5f);
                    float tubeVal = (n1 * n1) + (n2 * n2);

                    bool isCave = tubeVal < CAVE_DENSITY_THRESHOLD;

                    if (isCave)
                    {
                        blocks[x][y][z] = BlockID::AIR;
                        isSolidBlock[x][y][z] = false;
                    }
                    else
                    {
                        blocks[x][y][z] = BlockID::STONE;
                        isSolidBlock[x][y][z] = true;
                    }
                }
            }

            // Surface Decoration Pass: Apply grass, dirt, and steep cliff stone exposure
            int dirtDepth = 3 + GetSurfaceNoise(static_cast<int>(worldX), static_cast<int>(worldZ), seed);
            int hL = heightMap[x][z + 1];
            int hR = heightMap[x + 2][z + 1];
            int hD = heightMap[x + 1][z];
            int hU = heightMap[x + 1][z + 2];
            int slope = std::max({std::abs(surfaceHeight - hL), std::abs(surfaceHeight - hR),
                                  std::abs(surfaceHeight - hD), std::abs(surfaceHeight - hU)});

            for (int y = CHUNK_HEIGHT - 1; y >= 0; y--)
            {
                if (isSolidBlock[x][y][z] && blocks[x][y][z] == BlockID::STONE)
                {
                    blocks[x][y][z] = (slope > 2) ? BlockID::STONE : BlockID::GRASS;

                    for (int dy = 1; dy <= dirtDepth && (y - dy) >= 0; dy++)
                    {
                        if (blocks[x][y - dy][z] == BlockID::STONE)
                        {
                            blocks[x][y - dy][z] = (slope > 2) ? BlockID::STONE : BlockID::DIRT;
                        }
                        else
                        {
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }

    // Pass 2: Ore Vein Generator (Spawns strictly embedded inside solid STONE blocks)
    auto placeOreVein = [&](BlockID oreId, int veins, int size, int minY, int maxY, int salt)
    {
        ChunkRNG rng(chunkX, chunkZ, salt, seed);
        for (int i = 0; i < veins; i++)
        {
            float xCoord = static_cast<float>(rng.NextInt(CHUNK_WIDTH));
            float yCoord = static_cast<float>(minY + rng.NextInt(maxY - minY));
            float zCoord = static_cast<float>(rng.NextInt(CHUNK_DEPTH));

            for (int j = 0; j < size; j++)
            {
                float angle1 = rng.NextFloat() * 3.14159f * 2.0f;
                float yOffset = (rng.NextFloat() - 0.5f) * 2.0f;

                xCoord += sinf(angle1) * cosf(yOffset);
                yOffset += (rng.NextFloat() - 0.5f) * 0.5f;
                zCoord += cosf(angle1) * cosf(yOffset);

                int bx = static_cast<int>(roundf(xCoord));
                int by = static_cast<int>(roundf(yCoord));
                int bz = static_cast<int>(roundf(zCoord));

                if (bx >= 0 && bx < CHUNK_WIDTH && by >= minY && by < maxY && bz >= 0 && bz < CHUNK_DEPTH)
                {
                    if (blocks[bx][by][bz] == BlockID::STONE)
                    {
                        blocks[bx][by][bz] = oreId;
                    }
                }
            }
        }
    };

    placeOreVein(BlockID::COAL_ORE, 8, 17, 5, 128, 101);
    placeOreVein(BlockID::IRON_ORE, 6, 8, 5, 64, 102);
    placeOreVein(BlockID::GOLD_ORE, 2, 8, 5, 32, 103);
    placeOreVein(BlockID::LAPIS_ORE, 2, 6, 5, 32, 104);
    placeOreVein(BlockID::REDSTONE_ORE, 3, 8, 5, 16, 105);
    placeOreVein(BlockID::DIAMOND_ORE, 1, 7, 5, 16, 106);
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

// BuildMesh: Generates vertex data and initializes the GPU Mesh object
void Chunk::BuildMesh(const World &world)
{
    std::vector<float> vertices = GenerateMeshData(world);
    m_quadCount = vertices.size() / (7 * 6);
    if (!vertices.empty())
    {
        // Creates unique_ptr to Mesh, uploading data via buffer orphaning
        m_mesh = std::make_unique<Mesh>(vertices.data(), vertices.size() * sizeof(float), vertices.size() / 7);
    }
    else
    {
        m_mesh.reset();
    }
}

void Chunk::Draw() const
{
    if (m_mesh)
    {
        m_mesh->Draw();
    }
}

// GenerateMeshData: Performs greedy/face-culled vertex generation for visible block faces
std::vector<float> Chunk::GenerateMeshData(const World &world) const
{
    std::vector<float> vertices;
    // Pre-reserve memory to eliminate dynamic heap reallocations during push_back
    vertices.reserve(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH * 6 * 6 * 7 / 5);

    auto isSolid = [](BlockID id) { return id != BlockID::AIR; };

    auto getNeighborBlock = [&](int lx, int ly, int lz) -> BlockID
    {
        if (ly < 0 || ly >= CHUNK_HEIGHT) return BlockID::AIR;
        if (lx >= 0 && lx < CHUNK_WIDTH && lz >= 0 && lz < CHUNK_DEPTH)
        {
            return blocks[lx][ly][lz];
        }
        int worldX = m_chunkX * CHUNK_WIDTH + lx;
        int worldZ = m_chunkZ * CHUNK_DEPTH + lz;
        return world.GetBlock(worldX, ly, worldZ);
    };

    // Iterate through all blocks in chunk to generate visible geometry (face culling)
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
                if (!isSolid(getNeighborBlock(x + 1, y, z)))
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
                if (!isSolid(getNeighborBlock(x - 1, y, z)))
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
                if (!isSolid(getNeighborBlock(x, y + 1, z)))
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
                if (!isSolid(getNeighborBlock(x, y - 1, z)))
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
                if (!isSolid(getNeighborBlock(x, y, z + 1)))
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
                if (!isSolid(getNeighborBlock(x, y, z - 1)))
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