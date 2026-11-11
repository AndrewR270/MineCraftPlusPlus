#ifndef CHUNK_H
#define CHUNK_H

#include "Block.h"
#include <vector>

const int CHUNK_WIDTH = 16;
const int CHUNK_HEIGHT = 16;
const int CHUNK_DEPTH = 16;

class Chunk {
public:
    // 3D block storage array
    BlockID blocks[CHUNK_WIDTH][CHUNK_HEIGHT][CHUNK_DEPTH];

    Chunk();

    BlockID GetBlock(int x, int y, int z) const;
    void SetBlock(int x, int y, int z, BlockID id);

    // Generates a single dynamic vertex array with face culling applied
    std::vector<float> GenerateMesh() const;
};

#endif