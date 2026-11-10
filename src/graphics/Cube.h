#ifndef CUBE_H
#define CUBE_H

#include "../world/BlockRegistry.h"
#include <vector>

namespace Graphics {
    // Generates 36 vertices (7 floats each: Pos(3), Shading(1), UV(2), Tint(1)) for a single block
    inline std::vector<float> GenerateBlockVertices(BlockID blockId) {
        const BlockData& data = BlockRegistry::GetInstance().GetBlock(blockId);
        float tintFlag = data.hasGrassTint ? 1.0f : 0.0f;

        UVCoord t = data.top;
        UVCoord b = data.bottom;
        UVCoord s = data.side;

        std::vector<float> vertices = {
            // Right face (Shading: 0.6)
             0.5f,  0.5f,  0.5f,  0.6f,  s.uMax, s.vMax, 0.0f,
             0.5f, -0.5f,  0.5f,  0.6f,  s.uMax, s.vMin, 0.0f,
             0.5f, -0.5f, -0.5f,  0.6f,  s.uMin, s.vMin, 0.0f,
             0.5f, -0.5f, -0.5f,  0.6f,  s.uMin, s.vMin, 0.0f,
             0.5f,  0.5f, -0.5f,  0.6f,  s.uMin, s.vMax, 0.0f,
             0.5f,  0.5f,  0.5f,  0.6f,  s.uMax, s.vMax, 0.0f,

            // Left face (Shading: 0.6)
            -0.5f,  0.5f, -0.5f,  0.6f,  s.uMax, s.vMax, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.6f,  s.uMax, s.vMin, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.6f,  s.uMin, s.vMin, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.6f,  s.uMin, s.vMin, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.6f,  s.uMin, s.vMax, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.6f,  s.uMax, s.vMax, 0.0f,

            // Top face (Shading: 1.0)
             0.5f,  0.5f,  0.5f,  1.0f,  t.uMax, t.vMax, tintFlag,
             0.5f,  0.5f, -0.5f,  1.0f,  t.uMax, t.vMin, tintFlag,
            -0.5f,  0.5f, -0.5f,  1.0f,  t.uMin, t.vMin, tintFlag,
            -0.5f,  0.5f, -0.5f,  1.0f,  t.uMin, t.vMin, tintFlag,
            -0.5f,  0.5f,  0.5f,  1.0f,  t.uMin, t.vMax, tintFlag,
             0.5f,  0.5f,  0.5f,  1.0f,  t.uMax, t.vMax, tintFlag,

            // Bottom face (Shading: 0.4)
            -0.5f, -0.5f,  0.5f,  0.4f,  b.uMax, b.vMax, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.4f,  b.uMax, b.vMin, 0.0f,
             0.5f, -0.5f, -0.5f,  0.4f,  b.uMin, b.vMin, 0.0f,
             0.5f, -0.5f, -0.5f,  0.4f,  b.uMin, b.vMin, 0.0f,
             0.5f, -0.5f,  0.5f,  0.4f,  b.uMin, b.vMax, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.4f,  b.uMax, b.vMax, 0.0f,

            // Front face (Shading: 0.8)
            -0.5f,  0.5f,  0.5f,  0.8f,  s.uMin, s.vMax, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.8f,  s.uMin, s.vMin, 0.0f,
             0.5f, -0.5f,  0.5f,  0.8f,  s.uMax, s.vMin, 0.0f,
             0.5f, -0.5f,  0.5f,  0.8f,  s.uMax, s.vMin, 0.0f,
             0.5f,  0.5f,  0.5f,  0.8f,  s.uMax, s.vMax, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.8f,  s.uMin, s.vMax, 0.0f,

            // Back face (Shading: 0.8)
             0.5f,  0.5f, -0.5f,  0.8f,  s.uMin, s.vMax, 0.0f,
             0.5f, -0.5f, -0.5f,  0.8f,  s.uMin, s.vMin, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.8f,  s.uMax, s.vMin, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.8f,  s.uMax, s.vMin, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.8f,  s.uMax, s.vMax, 0.0f,
             0.5f,  0.5f, -0.5f,  0.8f,  s.uMin, s.vMax, 0.0f
        };

        return vertices;
    }
}

#endif