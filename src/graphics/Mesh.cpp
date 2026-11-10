#include "Mesh.h"

Mesh::Mesh(const float* vertices, unsigned int dataSize, unsigned int count) {
    vertexCount = count;

    // 1. Generate buffers and arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // 2. Bind Vertex Array Object
    glBindVertexArray(VAO);

    // 3. Copy vertex data into VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, dataSize, vertices, GL_STATIC_DRAW);

    // 4. Set vertex attribute pointers (position attribute at location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 5. Unbind VAO to prevent accidental corruption of state
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