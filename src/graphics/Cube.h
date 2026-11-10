#ifndef CUBE_H
#define CUBE_H

namespace Graphics {
    // 36 vertices: Position (3), Shading (1), UVs (2), GrassTint Flag (1)
    // Total stride = 7 floats per vertex
    const float CUBE_VERTICES[] = {
        // Right face (Grass side texture, no tint: 0.0)
         0.5f,  0.5f,  0.5f,  0.6f,  0.25f, 1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.6f,  0.25f, 0.9375f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.6f,  0.1875f, 0.9375f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.6f,  0.1875f, 0.9375f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.6f,  0.1875f, 1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.6f,  0.25f, 1.0f,  0.0f,

        // Left face (Grass side texture, no tint: 0.0)
        -0.5f,  0.5f, -0.5f,  0.6f,  0.25f, 1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.6f,  0.25f, 0.9375f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.6f,  0.1875f, 0.9375f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.6f,  0.1875f, 0.9375f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.6f,  0.1875f, 1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.6f,  0.25f, 1.0f,  0.0f,

        // Top face (Grass top texture, WITH biome tint: 1.0)
         0.5f,  0.5f,  0.5f,  1.0f,  0.0625f, 1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0625f, 0.9375f, 1.0f,
        -0.5f,  0.5f, -0.5f,  1.0f,  0.0f, 0.9375f,  1.0f,
        -0.5f,  0.5f, -0.5f,  1.0f,  0.0f, 0.9375f,  1.0f,
        -0.5f,  0.5f,  0.5f,  1.0f,  0.0f, 1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0625f, 1.0f,  1.0f,

        // Bottom face (Dirt texture, no tint: 0.0)
        -0.5f, -0.5f,  0.5f,  0.4f,  0.1875f, 1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.4f,  0.1875f, 0.9375f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.4f,  0.125f, 0.9375f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.4f,  0.125f, 0.9375f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.4f,  0.125f, 1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.4f,  0.1875f, 1.0f,  0.0f,

        // Front face (Grass side texture, no tint: 0.0)
        -0.5f,  0.5f,  0.5f,  0.8f,  0.1875f, 1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.8f,  0.1875f, 0.9375f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.8f,  0.25f, 0.9375f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.8f,  0.25f, 0.9375f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.8f,  0.25f, 1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.8f,  0.1875f, 1.0f,  0.0f,

        // Back face (Grass side texture, no tint: 0.0)
         0.5f,  0.5f, -0.5f,  0.8f,  0.1875f, 1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.8f,  0.1875f, 0.9375f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.8f,  0.25f, 0.9375f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.8f,  0.25f, 0.9375f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.8f,  0.25f, 1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.8f,  0.1875f, 1.0f,  0.0f
    };

    const unsigned int CUBE_VERTEX_COUNT = 36;
}

#endif