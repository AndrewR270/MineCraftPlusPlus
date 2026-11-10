#ifndef BLOCK_REGISTRY_H
#define BLOCK_REGISTRY_H

#include "Block.h"
#include <unordered_map>

class BlockRegistry {
private:
    std::unordered_map<BlockID, BlockData> blocks;
    BlockRegistry();

public:
    static BlockRegistry& GetInstance();
    const BlockData& GetBlock(BlockID id) const;
};

#endif