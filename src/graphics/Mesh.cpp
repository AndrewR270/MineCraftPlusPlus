#include "Mesh.h"

// Constructor: Allocates GPU buffers and uploads vertex data safely and asynchronously
Mesh::Mesh(const float* vertices, unsigned int dataSize, unsigned int count) {
    vertexCount = count;

    // Generate OpenGL Vertex Array Object (VAO) and Vertex Buffer Object (VBO) handles
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Bind the VAO to store vertex attribute configurations
    glBindVertexArray(VAO);
    // Bind the VBO to the GL_ARRAY_BUFFER target
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // ========================================================================
    // ARCHITECTURAL CONCEPT: BUFFER ORPHANING (Eliminating CPU-GPU Pipeline Stalls)
    // ========================================================================
    // WHY WE DO IT: Normally, calling glBufferData while the GPU is still rendering 
    // the previous frame forces the CPU to stall and wait for the GPU to finish reading 
    // the old buffer memory (causing micro-stutters). 
    // HOW IT WORKS: By passing `nullptr` with the desired buffer size first, we tell the 
    // OpenGL driver to "orphan" the old memory block. The driver immediately allocates a 
    // brand new memory region behind the scenes (allowing the GPU to finish reading the old 
    // block asynchronously without blocking), and `glBufferSubData` populates the new block 
    // instantly without pipeline bubbles.
    // ========================================================================
    glBufferData(GL_ARRAY_BUFFER, dataSize, nullptr, GL_DYNAMIC_DRAW);
    
    // Upload vertex data into the newly orphaned buffer storage
    glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, vertices);

    size_t stride = 7 * sizeof(float);

    // Define vertex attribute pointers so the GPU shader knows how to unpack vertex data:
    // 1. Position attribute (Location 0) - vec3
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // 2. Brightness / AO attribute (Location 1) - float
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 3. Texture Coordinates UV attribute (Location 2) - vec2
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // 4. Grass Tint Flag attribute (Location 3) - float
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // Unbind VAO to prevent accidental state pollution
    glBindVertexArray(0);
}

// Destructor: RAII resource cleanup ensuring zero GPU memory leaks
Mesh::~Mesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void Mesh::Draw() const {
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
}