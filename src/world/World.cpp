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
void World::UpdateStreaming(int playerChunkX, int playerChunkZ, int renderDistance) {
    std::vector<std::pair<int, int>> toUnload;

    // 1. Identify chunks outside render distance for garbage collection
    for (const auto& [coord, chunk] : m_chunks) {
        int cx = coord.first;
        int cz = coord.second;
        if (std::abs(cx - playerChunkX) > renderDistance || std::abs(cz - playerChunkZ) > renderDistance) {
            toUnload.push_back(coord);
        }
    }

    // Erase distant chunks (RAII automatically frees Chunk memory and GPU buffers via destructors)
    for (const auto& coord : toUnload) {
        m_chunks.erase(coord);
    }

    // 2. Gather all missing chunks within render distance into a temporary vector
    std::vector<std::pair<int, int>> missingChunks;
    for (int x = -renderDistance; x <= renderDistance; x++) {
        for (int z = -renderDistance; z <= renderDistance; z++) {
            int cx = playerChunkX + x;
            int cz = playerChunkZ + z;
            std::pair<int, int> coord = {cx, cz};

            // Enforce a circular render boundary (x^2 + z^2 <= r^2)
            if (x * x + z * z <= renderDistance * renderDistance) {
                // Check against loaded map and O(1) generating set to prevent duplicate queue work
                if (m_chunks.find(coord) == m_chunks.end() && m_generating.find(coord) == m_generating.end()) {
                    missingChunks.push_back(coord);
                }
            }
        }
    }

    // ========================================================================
    // ARCHITECTURAL CONCEPT: RADIAL DISTANCE PRIORITY SORTING
    // ========================================================================
    // WHY WE DO IT: Standard grid loops populate queues in row-major order, causing chunks 
    // to load in irregular directions. This tears open visual voids that expose underground caves.
    // HOW IT WORKS: We calculate Euclidean distance squared (`dx^2 + dz^2`) from the player 
    // for each missing chunk and sort them ascending. This forces the engine to build 
    // concentric outward circles, ensuring inner protective shells are built before outer rings.
    // ========================================================================
    std::sort(missingChunks.begin(), missingChunks.end(), [playerChunkX, playerChunkZ](const auto& a, const auto& b) {
        int distA = (a.first - playerChunkX) * (a.first - playerChunkX) + (a.second - playerChunkZ) * (a.second - playerChunkZ);
        int distB = (b.first - playerChunkX) * (b.first - playerChunkX) + (b.second - playerChunkZ) * (b.second - playerChunkZ);
        return distA < distB; // Closest chunks processed first
    });

    // 4. Push sorted chunks into the generation queue and O(1) tracking set
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
            break; // Time budget exhausted for this frame; yield control!
        }

        m_genQueue.pop();
        m_generating.erase(coord); // Remove from O(1) tracking set

        // Verify chunk wasn't unloaded while sitting in queue
        if (m_chunks.find(coord) == m_chunks.end()) {
            m_chunks[coord] = std::make_unique<Chunk>(coord.first, coord.second, m_seed);
            m_meshQueue.push(coord); // Forward generated chunk to mesh upload queue
        }
    }

    // 2. Process Mesh Upload Queue (GPU VBO Uploads)
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
            break; // Time budget exhausted for GPU uploads this frame
        }

        m_meshQueue.pop();
        chunk->BuildMesh(*this);

        // Refresh immediate neighbor meshes so cross-chunk face culling updates correctly
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