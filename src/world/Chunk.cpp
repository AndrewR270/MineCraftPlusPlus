#include "Chunk.h"
#include "BlockRegistry.h"

Chunk::Chunk() {
    // Populate a test terrain matching your 16x64x16 grid:
    // Stone base (0-30), Dirt middle (31-62), Grass top (63)
    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int z = 0; z < CHUNK_DEPTH; z++) {
            for (int y = 0; y < CHUNK_HEIGHT; y++) {
                if (y < 30)
                    blocks[x][y][z] = BlockID::STONE;
                else if (y < 63)
                    blocks[x][y][z] = BlockID::DIRT;
                else if (y == 63)
                    blocks[x][y][z] = BlockID::GRASS;
                else
                    blocks[x][y][z] = BlockID::AIR;
            }
        }
    }
}

BlockID Chunk::GetBlock(int x, int y, int z) const {
    if (x < 0 || x >= CHUNK_WIDTH || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_DEPTH)
        return BlockID::AIR; // Out-of-bounds acts as open air
    return blocks[x][y][z];
}

void Chunk::SetBlock(int x, int y, int z, BlockID id) {
    if (x >= 0 && x < CHUNK_WIDTH && y >= 0 && y < CHUNK_HEIGHT && z >= 0 && z < CHUNK_DEPTH) {
        blocks[x][y][z] = id;
    }
}

std::vector<float> Chunk::GenerateMesh() const {
    std::vector<float> vertices;

    auto isSolid = [](BlockID id) {
        return id != BlockID::AIR;
    };

    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            for (int z = 0; z < CHUNK_DEPTH; z++) {
                BlockID currentId = blocks[x][y][z];
                if (currentId == BlockID::AIR) continue;

                const BlockData& data = BlockRegistry::GetInstance().GetBlock(currentId);
                float tintFlag = data.hasGrassTint ? 1.0f : 0.0f;
                UVCoord t = data.top;
                UVCoord b = data.bottom;
                UVCoord s = data.side;

                // 1. Right Face (+X)
                if (!isSolid(GetBlock(x + 1, y, z))) {
                    float faceVerts[] = {
                        x + 0.5f,  y + 0.5f,  z + 0.5f,  0.6f,  s.uMax, s.vMax, 0.0f,
                        x + 0.5f,  y - 0.5f,  z + 0.5f,  0.6f,  s.uMax, s.vMin, 0.0f,
                        x + 0.5f,  y - 0.5f,  z - 0.5f,  0.6f,  s.uMin, s.vMin, 0.0f,
                        x + 0.5f,  y - 0.5f,  z - 0.5f,  0.6f,  s.uMin, s.vMin, 0.0f,
                        x + 0.5f,  y + 0.5f,  z - 0.5f,  0.6f,  s.uMin, s.vMax, 0.0f,
                        x + 0.5f,  y + 0.5f,  z + 0.5f,  0.6f,  s.uMax, s.vMax, 0.0f
                    };
                    vertices.insert(vertices.end(), std::begin(faceVerts), std::end(faceVerts));
                }

                // 2. Left Face (-X)
                if (!isSolid(GetBlock(x - 1, y, z))) {
                    float faceVerts[] = {
                        x - 0.5f,  y + 0.5f,  z - 0.5f,  0.6f,  s.uMax, s.vMax, 0.0f,
                        x - 0.5f,  y - 0.5f,  z - 0.5f,  0.6f,  s.uMax, s.vMin, 0.0f,
                        x - 0.5f,  y - 0.5f,  z + 0.5f,  0.6f,  s.uMin, s.vMin, 0.0f,
                        x - 0.5f,  y - 0.5f,  z + 0.5f,  0.6f,  s.uMin, s.vMin, 0.0f,
                        x - 0.5f,  y + 0.5f,  z + 0.5f,  0.6f,  s.uMin, s.vMax, 0.0f,
                        x - 0.5f,  y + 0.5f,  z - 0.5f,  0.6f,  s.uMax, s.vMax, 0.0f
                    };
                    vertices.insert(vertices.end(), std::begin(faceVerts), std::end(faceVerts));
                }

                // 3. Top Face (+Y)
                if (!isSolid(GetBlock(x, y + 1, z))) {
                    float faceVerts[] = {
                        x + 0.5f,  y + 0.5f,  z + 0.5f,  1.0f,  t.uMax, t.vMax, tintFlag,
                        x + 0.5f,  y + 0.5f,  z - 0.5f,  1.0f,  t.uMax, t.vMin, tintFlag,
                        x - 0.5f,  y + 0.5f,  z - 0.5f,  1.0f,  t.uMin, t.vMin, tintFlag,
                        x - 0.5f,  y + 0.5f,  z - 0.5f,  1.0f,  t.uMin, t.vMin, tintFlag,
                        x - 0.5f,  y + 0.5f,  z + 0.5f,  1.0f,  t.uMin, t.vMax, tintFlag,
                        x + 0.5f,  y + 0.5f,  z + 0.5f,  1.0f,  t.uMax, t.vMax, tintFlag
                    };
                    vertices.insert(vertices.end(), std::begin(faceVerts), std::end(faceVerts));
                }

                // 4. Bottom Face (-Y)
                if (!isSolid(GetBlock(x, y - 1, z))) {
                    float faceVerts[] = {
                        x - 0.5f,  y - 0.5f,  z + 0.5f,  0.4f,  b.uMax, b.vMax, 0.0f,
                        x - 0.5f,  y - 0.5f,  z - 0.5f,  0.4f,  b.uMax, b.vMin, 0.0f,
                        x + 0.5f,  y - 0.5f,  z - 0.5f,  0.4f,  b.uMin, b.vMin, 0.0f,
                        x + 0.5f,  y - 0.5f,  z - 0.5f,  0.4f,  b.uMin, b.vMin, 0.0f,
                        x + 0.5f,  y - 0.5f,  z + 0.5f,  0.4f,  b.uMin, b.vMax, 0.0f,
                        x - 0.5f,  y - 0.5f,  z + 0.5f,  0.4f,  b.uMax, b.vMax, 0.0f
                    };
                    vertices.insert(vertices.end(), std::begin(faceVerts), std::end(faceVerts));
                }

                // 5. Front Face (+Z)
                if (!isSolid(GetBlock(x, y, z + 1))) {
                    float faceVerts[] = {
                        x - 0.5f,  y + 0.5f,  z + 0.5f,  0.8f,  s.uMin, s.vMax, 0.0f,
                        x - 0.5f,  y - 0.5f,  z + 0.5f,  0.8f,  s.uMin, s.vMin, 0.0f,
                        x + 0.5f,  y - 0.5f,  z + 0.5f,  0.8f,  s.uMax, s.vMin, 0.0f,
                        x + 0.5f,  y - 0.5f,  z + 0.5f,  0.8f,  s.uMax, s.vMin, 0.0f,
                        x + 0.5f,  y + 0.5f,  z + 0.5f,  0.8f,  s.uMax, s.vMax, 0.0f,
                        x - 0.5f,  y + 0.5f,  z + 0.5f,  0.8f,  s.uMin, s.vMax, 0.0f
                    };
                    vertices.insert(vertices.end(), std::begin(faceVerts), std::end(faceVerts));
                }

                // 6. Back Face (-Z)
                if (!isSolid(GetBlock(x, y, z - 1))) {
                    float faceVerts[] = {
                        x + 0.5f,  y + 0.5f,  z - 0.5f,  0.8f,  s.uMin, s.vMax, 0.0f,
                        x + 0.5f,  y - 0.5f,  z - 0.5f,  0.8f,  s.uMin, s.vMin, 0.0f,
                        x - 0.5f,  y - 0.5f,  z - 0.5f,  0.8f,  s.uMax, s.vMin, 0.0f,
                        x - 0.5f,  y - 0.5f,  z - 0.5f,  0.8f,  s.uMax, s.vMin, 0.0f,
                        x - 0.5f,  y + 0.5f,  z - 0.5f,  0.8f,  s.uMax, s.vMax, 0.0f,
                        x + 0.5f,  y + 0.5f,  z - 0.5f,  0.8f,  s.uMin, s.vMax, 0.0f
                    };
                    vertices.insert(vertices.end(), std::begin(faceVerts), std::end(faceVerts));
                }
            }
        }
    }

    return vertices;
}