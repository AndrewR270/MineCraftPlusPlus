#version 330 core
out vec4 FragColor;

in float Shading;
in vec2 TexCoord;
in float GrassTint;

uniform sampler2D texture1;

void main() {
    vec4 texColor = texture(texture1, TexCoord);
    
    // Classic Minecraft biome green tint color
    vec3 biomeGreen = vec3(0.48f, 0.77f, 0.31f);
    
    // Multiply texture color by biome tint if this is a grass top face
    vec3 baseColor = (GrassTint > 0.5f) ? (texColor.rgb * biomeGreen) : texColor.rgb;
    
    FragColor = vec4(baseColor * Shading, texColor.a);
}