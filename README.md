# MineCraftPlusPlus
A Minecraft Clone in C++.

# Setup Command

mkdir -p src/graphics src/world src/physics src/entity src/inventory src/gui assets/shaders assets/textures

Vendors:
GLFW 3.6
GLM 1.0.3
GLAD OpenGL 3.3 Core


GLFW: Creates the OS application window, manages the OpenGL context, captures raw hardware input (WASD, mouse offsets), and handles frame swapping.

GLAD: Loads physical OpenGL function pointers at runtime. Because GPU drivers vary by vendor (NVIDIA, AMD, Intel), GLAD maps function calls (like glGenBuffers) to driver memory addresses.

GLM: Provides 3D vector and matrix math math utilities (glm::mat4, glm::vec3, glm::perspective, glm::lookAt) matching GLSL shader syntax.

stb_image: Decodes image byte streams (.png, .jpg) into raw RGBA pixel arrays in memory so OpenGL can upload them to VRAM.

FastNoiseLite (FNL): Provides noise functions for terrain and cave generation.

static float WORLD_TERRAIN_FREQ       = 0.015f;  // Controls width of surface hills
static float CAVE_FREQ_XZ             = 0.015f;  // Controls horizontal scale and winding frequency of caves
static float CAVE_FREQ_Y              = 0.025f;  // Controls vertical scale of caves
static float CAVE_DENSITY_THRESHOLD   = 0.005f;  // Controls tunnel thickness / radius squared
static int   SURFACE_BUFFER           = 20;      // Solid stone buffer beneath dirt to dome cave ceilings


### How to Fix It

To resolve the mismatch and fully integrate streaming hysteresis, update `World.h` and `World.cpp` to support the 4-argument signature.


<Step subtitle="Verification" title="Recompile and Run">
Clean and recompile your project. 
*Verification:* Launch the engine. Frame 1 will render the initial chunks instantly, and walking around will smoothly stream new chunks in concentric radial circles without compiler errors or empty voids.
</Step>
</Steps>

---

<ElicitationsGroup message="If you run into any further compiler warnings or build issues:">
  <Elicitation label="Check CMake or build flags" query="How do I configure CMake to catch signature mismatches and undefined references automatically?"/>
  <Elicitation label="Verify header inclusions" query="Which headers need to be included in World.h for unordered_set and vector?"/>
</ElicitationsGroup>

<FollowUp label="Want me to review the rest of your World class methods to ensure complete consistency?" query="Yes, review all methods in World.h and World.cpp for consistency and best practices."/>