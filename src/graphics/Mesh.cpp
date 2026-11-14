#include "Mesh.h"

Mesh::Mesh(const float* vertices, unsigned int dataSize, unsigned int count) {
    vertexCount = count;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // 1. Orphan the buffer memory asynchronously to prevent pipeline stalls
    glBufferData(GL_ARRAY_BUFFER, dataSize, nullptr, GL_DYNAMIC_DRAW);
    
    // 2. Upload vertex data into the allocated buffer
    glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, vertices);

    size_t stride = 7 * sizeof(float);

    // 1. Position attribute (Location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // 2. Brightness / AO attribute (Location 1)
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 3. Texture Coordinates UV attribute (Location 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // 4. Grass Tint Flag attribute (Location 3)
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void Mesh::Draw() const {
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
}