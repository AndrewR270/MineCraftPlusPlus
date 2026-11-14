#include "World.h"
#include <cmath>
#include <vector>
#include <chrono>
#include <algorithm> // Required for std::sort

World::World(unsigned int seed) : m_seed(seed) {}

// Converts world coordinates to local chunk and block indices
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

            if (m_chunks.find(coord) == m_chunks.end() && m_generating.find(coord) == m_generating.end()) {
                m_chunks[coord] = std::make_unique<Chunk>(cx, cz, m_seed);
                m_meshQueue.push(coord);
            }
        }
    }
    ProcessQueues(9999.0, 9999.0);
}

// UpdateStreaming: Handles dynamic chunk loading/unloading based on player position
void World::UpdateStreaming(int playerChunkX, int playerChunkZ, int loadRadius, int unloadRadius) {
    std::vector<std::pair<int, int>> toUnload;

    // 1. Unload chunks exceeding the wider UNLOAD radius
    for (const auto& [coord, chunk] : m_chunks) {
        int cx = coord.first;
        int cz = coord.second;
        if (std::abs(cx - playerChunkX) > unloadRadius || std::abs(cz - playerChunkZ) > unloadRadius) {
            toUnload.push_back(coord);
        }
    }

    for (const auto& coord : toUnload) {
        m_chunks.erase(coord);
    }

    // 2. Gather missing chunks within the LOAD radius
    std::vector<std::pair<int, int>> missingChunks;
    for (int x = -loadRadius; x <= loadRadius; x++) {
        for (int z = -loadRadius; z <= loadRadius; z++) {
            int cx = playerChunkX + x;
            int cz = playerChunkZ + z;
            std::pair<int, int> coord = {cx, cz};

            if (x * x + z * z <= loadRadius * loadRadius) {
                if (m_chunks.find(coord) == m_chunks.end() && m_generating.find(coord) == m_generating.end()) {
                    missingChunks.push_back(coord);
                }
            }
        }
    }

    // 3. Sort strictly by radial distance from the player
    std::sort(missingChunks.begin(), missingChunks.end(), [playerChunkX, playerChunkZ](const auto& a, const auto& b) {
        int distA = (a.first - playerChunkX) * (a.first - playerChunkX) + (a.second - playerChunkZ) * (a.second - playerChunkZ);
        int distB = (b.first - playerChunkX) * (b.first - playerChunkX) + (b.second - playerChunkZ) * (b.second - playerChunkZ);
        return distA < distB;
    });

    for (const auto& coord : missingChunks) {
        m_generating.insert(coord);
        m_genQueue.push(coord);
    }
}

// ProcessQueues: Time-budgeted asynchronous task scheduler
void World::ProcessQueues(double maxGenTimeMs, double maxMeshTimeMs) {
    // ========================================================================
    // ARCHITECTURAL CONCEPT: CHRONO TIME-BUDGETED SCHEDULING
    // ========================================================================
    // WHY WE DO IT: Generating 441 chunks in a single frame freezes the game for 1 second.
    // HOW IT WORKS: We capture a high-resolution timestamp using `std::chrono::high_resolution_clock`.
    // As we process queue items, we check elapsed milliseconds each iteration. The moment our 
    // time budget (e.g., 1.5ms) is reached, we `break` out of the loop and yield execution back 
    // to the render loop, spreading heavy workloads across multiple frames seamlessly.
    // ========================================================================
    auto genStart = std::chrono::high_resolution_clock::now();

    // 1. Process Chunk Generation Queue
    while (!m_genQueue.empty()) {
        auto coord = m_genQueue.front();

        auto now = std::chrono::high_resolution_clock::now();
        double elapsedGen = std::chrono::duration<double, std::milli>(now - genStart).count();
        if (elapsedGen >= maxGenTimeMs) {
            break; 
        }

        m_genQueue.pop();
        m_generating.erase(coord);

        if (m_chunks.find(coord) == m_chunks.end()) {
            m_chunks[coord] = std::make_unique<Chunk>(coord.first, coord.second, m_seed);
            m_meshQueue.push(coord);
        }
    }

    // 2. Process Mesh Upload Queue (Drains queue and builds GPU VBOs)
    auto meshStart = std::chrono::high_resolution_clock::now();
    while (!m_meshQueue.empty()) {
        auto coord = m_meshQueue.front();
        
        Chunk* chunk = GetChunk(coord.first, coord.second);
        if (!chunk) {
            m_meshQueue.pop();
            continue;
        }

        auto now = std::chrono::high_resolution_clock::now();
        double elapsedMesh = std::chrono::duration<double, std::milli>(now - meshStart).count();
        if (elapsedMesh >= maxMeshTimeMs) {
            break; 
        }

        m_meshQueue.pop();
        chunk->BuildMesh(*this);

        int cx = chunk->GetChunkX();
        int cz = chunk->GetChunkZ();
        if (Chunk* n = GetChunk(cx - 1, cz)) n->BuildMesh(*this);
        if (Chunk* n = GetChunk(cx + 1, cz)) n->BuildMesh(*this);
        if (Chunk* n = GetChunk(cx, cz - 1)) n->BuildMesh(*this);
        if (Chunk* n = GetChunk(cx, cz + 1)) n->BuildMesh(*this);
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