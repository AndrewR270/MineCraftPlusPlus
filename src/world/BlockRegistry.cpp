#include "BlockRegistry.h"

BlockRegistry::BlockRegistry() {
    // Helper to calculate UVs from 16x16 atlas grid coordinates (col, row)
    auto getTileUV = [](int col, int row) -> UVCoord {
        float size = 0.0625f; // 1.0 / 16.0
        return {
            col * size,          // uMin
            1.0f - ((row + 1) * size), // vMin (inverted Y for OpenGL)
            (col + 1) * size,    // uMax
            1.0f - (row * size)  // vMax
        };
    };

    // 1. Dirt Block (col 2, row 0)
    UVCoord dirtUV = getTileUV(2, 0);
    blocks[BlockID::DIRT] = {
        BlockID::DIRT, "Dirt", false, dirtUV, dirtUV, dirtUV
    };

    // 2. Grass Block (Top: col 0, row 0; Bottom: dirt; Side: col 3, row 0)
    blocks[BlockID::GRASS] = {
        BlockID::GRASS, "Grass", true,
        getTileUV(0, 0), // Top (grayscale biome tint applied)
        dirtUV,          // Bottom
        getTileUV(3, 0)  // Side
    };

    // 3. Stone Block (col 1, row 0)
    UVCoord stoneUV = getTileUV(1, 0);
    blocks[BlockID::STONE] = {
        BlockID::STONE, "Stone", false, stoneUV, stoneUV, stoneUV
    };

    // 4. Bedrock Block (col 1, row 1)
    UVCoord bedrockUV = getTileUV(1, 1);
    blocks[BlockID::BEDROCK] = {
        BlockID::BEDROCK, "Bedrock", false, bedrockUV, bedrockUV, bedrockUV
    };
}

BlockRegistry& BlockRegistry::GetInstance() {
    static BlockRegistry instance;
    return instance;
}

const BlockData& BlockRegistry::GetBlock(BlockID id) const {
    return blocks.at(id);
}