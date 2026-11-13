#ifndef CHUNK_H
#define CHUNK_H

#include "Block.h"
#include "graphics/Mesh.h"
#include <vector>
#include <memory>

class World;

const int CHUNK_WIDTH = 16;
const int CHUNK_HEIGHT = 128; // Java 1.0 standard height limit
const int CHUNK_DEPTH = 16;

class Chunk {
public:
    BlockID blocks[CHUNK_WIDTH][CHUNK_HEIGHT][CHUNK_DEPTH];

    Chunk(int chunkX, int chunkZ, unsigned int seed);

    BlockID GetBlock(int x, int y, int z) const;
    void SetBlock(int x, int y, int z, BlockID id);

    // Generates mesh using World for cross-chunk face culling and caches it on the GPU
    void BuildMesh(const World& world);
    void Draw() const;

    int GetChunkX() const { return m_chunkX; }
    int GetChunkZ() const { return m_chunkZ; }
    size_t GetQuadCount() const { return m_quadCount; }

private:
    int m_chunkX;
    int m_chunkZ;
    std::unique_ptr<Mesh> m_mesh;
    size_t m_quadCount = 0;

    std::vector<float> GenerateMeshData(const World& world) const;
};

#endif