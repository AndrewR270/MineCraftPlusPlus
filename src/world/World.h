#pragma once
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <queue>
#include <utility>
#include "Chunk.h"

struct ChunkCoordHash {
    template <class T1, class T2>
    size_t operator()(const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

using ChunkMap = std::unordered_map<std::pair<int, int>, std::unique_ptr<Chunk>, ChunkCoordHash>;
using ChunkSet = std::unordered_set<std::pair<int, int>, ChunkCoordHash>;

class World {
public:
    World(unsigned int seed);
    ~World() = default;

    void LoadArea(int centerChunkX, int centerChunkZ, int radius);
    void UpdateStreaming(int playerChunkX, int playerChunkZ, int loadRadius, int unloadRadius);
    void ProcessQueues(double maxGenTimeMs, double maxMeshTimeMs);

    BlockID GetBlock(int x, int y, int z) const;
    void SetBlock(int x, int y, int z, BlockID id);

    Chunk* GetChunk(int cx, int cz) const;
    const ChunkMap& GetLoadedChunks() const { return m_chunks; }
    size_t GetTotalQuads() const;

    unsigned int GetSeed() const { return m_seed; }

private:
    unsigned int m_seed;
    ChunkMap m_chunks;
    
    std::queue<std::pair<int, int>> m_genQueue;
    ChunkSet m_generating;                        // Tracks items currently in m_genQueue
    
    std::queue<std::pair<int, int>> m_meshQueue;
    ChunkSet m_meshing;                           // Optional: Tracks items currently in m_meshQueue

    void GlobalToLocal(int worldX, int worldY, int worldZ, int& outCx, int& outCz, int& outLocalX, int& outLocalZ) const;
};