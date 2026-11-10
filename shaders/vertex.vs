#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aShading;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in float aGrassTint;

out float Shading;
out vec2 TexCoord;
out float GrassTint;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    Shading = aShading;
    TexCoord = aTexCoord;
    GrassTint = aGrassTint;
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
}