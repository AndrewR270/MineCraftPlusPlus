# MinecraftPlusPlus
A Minecraft Clone in C++.

# Setup Command

mkdir -p src/graphics src/world src/physics src/entity src/inventory src/gui assets/shaders assets/textures

Vendors:
GLFW 3.6
GLM 1.0.3
GLAD OpenGL 3.3 Core


GLFW: Creates the OS application window, manages the OpenGL context, captures raw hardware input (WASD, mouse offsets), and handles frame swapping.

GLAD: Loads physical OpenGL function pointers at runtime. Because GPU drivers vary by vendor (NVIDIA, AMD, Intel), GLAD maps function calls (like glGenBuffers) to driver memory addresses.

GLM:Provides 3D vector and matrix math math utilities (glm::mat4, glm::vec3, glm::perspective, glm::lookAt) matching GLSL shader syntax.

stb_image: Decodes image byte streams (.png, .jpg) into raw RGBA pixel arrays in memory so OpenGL can upload them to VRAM.