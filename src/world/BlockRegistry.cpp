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

    // 5. Coal Ore (col 2, row 2)
    UVCoord coalUV = getTileUV(2, 2);
    blocks[BlockID::COAL_ORE] = { BlockID::COAL_ORE, "Coal Ore", false, coalUV, coalUV, coalUV };

    // 6. Iron Ore (col 1, row 2)
    UVCoord ironUV = getTileUV(1, 2);
    blocks[BlockID::IRON_ORE] = { BlockID::IRON_ORE, "Iron Ore", false, ironUV, ironUV, ironUV };

    // 7. Gold Ore (col 0, row 2)
    UVCoord goldUV = getTileUV(0, 2);
    blocks[BlockID::GOLD_ORE] = { BlockID::GOLD_ORE, "Gold Ore", false, goldUV, goldUV, goldUV };

    // 8. Lapis Ore (col 0, row 10)
    UVCoord lapisUV = getTileUV(0, 10);
    blocks[BlockID::LAPIS_ORE] = { BlockID::LAPIS_ORE, "Lapis Ore", false, lapisUV, lapisUV, lapisUV };

    // 9. Diamond Ore (col 2, row 3)
    UVCoord diamondUV = getTileUV(2, 3);
    blocks[BlockID::DIAMOND_ORE] = { BlockID::DIAMOND_ORE, "Diamond Ore", false, diamondUV, diamondUV, diamondUV };

    // 10. Redstone Ore (col 3, row 3)
    UVCoord redstoneUV = getTileUV(3, 3);
    blocks[BlockID::REDSTONE_ORE] = { BlockID::REDSTONE_ORE, "Redstone Ore", false, redstoneUV, redstoneUV, redstoneUV };
}

BlockRegistry& BlockRegistry::GetInstance() {
    static BlockRegistry instance;
    return instance;
}

const BlockData& BlockRegistry::GetBlock(BlockID id) const {
    return blocks.at(id);
}