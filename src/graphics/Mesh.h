#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>

class Mesh {
public:
    unsigned int VAO, VBO;
    unsigned int vertexCount;

    // Constructor: Takes vertex data array, buffer size in bytes, and vertex count
    Mesh(const float* vertices, unsigned int dataSize, unsigned int count);
    
    // Destructor: Cleans up GPU buffers safely
    ~Mesh();

    // Renders the mesh
    void Draw() const;
};

#endif