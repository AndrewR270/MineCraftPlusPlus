#ifndef BLOCK_H
#define BLOCK_H

#include <glm.hpp>

enum class BlockID : uint8_t {
    AIR = 0,
    DIRT = 1,
    GRASS = 2,
    STONE = 3,
    BEDROCK = 4,
    COAL_ORE = 5,
    IRON_ORE = 6,
    GOLD_ORE = 7,
    LAPIS_ORE = 8,
    DIAMOND_ORE = 9,
    REDSTONE_ORE = 10
};

struct UVCoord {
    float uMin, vMin, uMax, vMax;
};

struct BlockData {
    BlockID id;
    const char* name;
    bool hasGrassTint;
    
    // Atlas UV coordinates per face direction
    UVCoord top;
    UVCoord bottom;
    UVCoord side;
};

#endif