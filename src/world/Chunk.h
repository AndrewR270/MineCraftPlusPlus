#ifndef CHUNK_H
#define CHUNK_H

#include "Block.h"
#include <vector>

const int CHUNK_WIDTH = 16;
const int CHUNK_HEIGHT = 256;
const int CHUNK_DEPTH = 16;

class Chunk {
public:
    // 3D array storing block IDs for this chunk column
    BlockID blocks[CHUNK_WIDTH][CHUNK_HEIGHT][CHUNK_DEPTH];

    Chunk();
    
    // Check bounds safely
    BlockID GetBlock(int x, int y, int z) const;
    void SetBlock(int x, int y, int z, BlockID id);

    // Generates vertex data with face culling optimization
    std::vector<float> GenerateMesh() const;
};

#endif