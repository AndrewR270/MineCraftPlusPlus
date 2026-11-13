#include "World.h"
#include <cmath>
#include <vector>

World::World(unsigned int seed) : m_seed(seed) {}

void World::GlobalToLocal(int worldX, int worldY, int worldZ, int& outCx, int& outCz, int& outLocalX, int& outLocalZ) const {
    outCx = (worldX >= 0) ? (worldX / CHUNK_WIDTH) : ((worldX - CHUNK_WIDTH + 1) / CHUNK_WIDTH);
    outCz = (worldZ >= 0) ? (worldZ / CHUNK_DEPTH) : ((worldZ - CHUNK_DEPTH + 1) / CHUNK_DEPTH);

    outLocalX = worldX - (outCx * CHUNK_WIDTH);
    outLocalZ = worldZ - (outCz * CHUNK_DEPTH);

    if (outLocalX < 0) outLocalX += CHUNK_WIDTH;
    if (outLocalZ < 0) outLocalZ += CHUNK_DEPTH;
}

void World::LoadArea(int centerChunkX, int centerChunkZ, int radius) {
    for (int x = -radius; x <= radius; x++) {
        for (int z = -radius; z <= radius; z++) {
            int cx = centerChunkX + x;
            int cz = centerChunkZ + z;
            std::pair<int, int> coord = {cx, cz};

            if (m_chunks.find(coord) == m_chunks.end()) {
                m_chunks[coord] = std::make_unique<Chunk>(cx, cz, m_seed);
                m_meshQueue.push(coord);
            }
        }
    }
    // Build all for initial load
    ProcessMeshQueue(static_cast<int>(m_meshQueue.size()));
}

void World::UpdateStreaming(int playerChunkX, int playerChunkZ, int renderDistance) {
    std::vector<std::pair<int, int>> toUnload;

    // 1. Identify chunks outside render distance
    for (const auto& [coord, chunk] : m_chunks) {
        int cx = coord.first;
        int cz = coord.second;
        if (std::abs(cx - playerChunkX) > renderDistance || std::abs(cz - playerChunkZ) > renderDistance) {
            toUnload.push_back(coord);
        }
    }

    // Erase distant chunks from memory
    for (const auto& coord : toUnload) {
        m_chunks.erase(coord);
    }

    // 2. Load missing chunk data instantly (fast CPU array allocation, no GPU stall)
    for (int x = -renderDistance; x <= renderDistance; x++) {
        for (int z = -renderDistance; z <= renderDistance; z++) {
            int cx = playerChunkX + x;
            int cz = playerChunkZ + z;
            std::pair<int, int> coord = {cx, cz};

            if (m_chunks.find(coord) == m_chunks.end()) {
                m_chunks[coord] = std::make_unique<Chunk>(cx, cz, m_seed);
                m_meshQueue.push(coord);
            }
        }
    }
}

void World::ProcessMeshQueue(int maxChunksPerFrame) {
    int processed = 0;
    while (!m_meshQueue.empty() && processed < maxChunksPerFrame) {
        auto coord = m_meshQueue.front();
        m_meshQueue.pop();

        Chunk* chunk = GetChunk(coord.first, coord.second);
        if (chunk) {
            chunk->BuildMesh(*this);

            int cx = chunk->GetChunkX();
            int cz = chunk->GetChunkZ();

            // Refresh adjacent neighbor meshes if needed for border culling
            if (Chunk* n = GetChunk(cx - 1, cz)) n->BuildMesh(*this);
            if (Chunk* n = GetChunk(cx + 1, cz)) n->BuildMesh(*this);
            if (Chunk* n = GetChunk(cx, cz - 1)) n->BuildMesh(*this);
            if (Chunk* n = GetChunk(cx, cz + 1)) n->BuildMesh(*this);
        }
        processed++;
    }
}

Chunk* World::GetChunk(int cx, int cz) const {
    auto it = m_chunks.find({cx, cz});
    if (it != m_chunks.end()) {
        return it->second.get();
    }
    return nullptr;
}

BlockID World::GetBlock(int x, int y, int z) const {
    if (y < 0 || y >= CHUNK_HEIGHT) return BlockID::AIR;

    int cx, cz, localX, localZ;
    GlobalToLocal(x, y, z, cx, cz, localX, localZ);

    Chunk* chunk = GetChunk(cx, cz);
    if (chunk) {
        return chunk->GetBlock(localX, y, localZ);
    }
    return BlockID::AIR;
}

void World::SetBlock(int x, int y, int z, BlockID id) {
    if (y < 0 || y >= CHUNK_HEIGHT) return;

    int cx, cz, localX, localZ;
    GlobalToLocal(x, y, z, cx, cz, localX, localZ);

    Chunk* chunk = GetChunk(cx, cz);
    if (chunk) {
        chunk->SetBlock(localX, y, localZ, id);
        chunk->BuildMesh(*this);

        if (localX == 0)                { if (Chunk* n = GetChunk(cx - 1, cz)) n->BuildMesh(*this); }
        if (localX == CHUNK_WIDTH - 1)  { if (Chunk* n = GetChunk(cx + 1, cz)) n->BuildMesh(*this); }
        if (localZ == 0)                { if (Chunk* n = GetChunk(cx, cz - 1)) n->BuildMesh(*this); }
        if (localZ == CHUNK_DEPTH - 1)  { if (Chunk* n = GetChunk(cx, cz + 1)) n->BuildMesh(*this); }
    }
}

size_t World::GetTotalQuads() const {
    size_t total = 0;
    for (const auto& [coord, chunk] : m_chunks) {
        total += chunk->GetQuadCount();
    }
    return total;
}