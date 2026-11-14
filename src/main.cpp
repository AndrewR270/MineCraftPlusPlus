#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include "graphics/Camera.h"
#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/Frustum.h"
#include "world/BlockRegistry.h"
#include "world/Chunk.h"
#include "world/World.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Window resize viewport callback
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Mouse movement callback for free-cam looking
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    Camera *camera = static_cast<Camera *>(glfwGetWindowUserPointer(window));
    if (camera)
    {
        camera->ProcessMouseMovement(xoffset, yoffset);
    }
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "MineCraftPlusPlus", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Disable V-Sync (0) to unlock frame rates for benchmarking
    glfwSwapInterval(0);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader ourShader("shaders/vertex.vs", "shaders/fragment.fs");

    // Initialize world seed using current system time
    unsigned int worldSeed = static_cast<unsigned int>(time(nullptr));
    srand(worldSeed);

    World world(worldSeed);

    // Load texture atlas
    Texture terrainAtlas("assets/textures/textures.png");
    ourShader.use();
    ourShader.setInt("texture1", 0);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    Camera camera(glm::vec3(8.0f, 85.0f, 25.0f));

    // 1. Calculate initial player chunk coordinates from starting position
    int pChunkX = (camera.Position.x >= 0) ? (static_cast<int>(camera.Position.x) / CHUNK_WIDTH) : ((static_cast<int>(camera.Position.x) - CHUNK_WIDTH + 1) / CHUNK_WIDTH);
    int pChunkZ = (camera.Position.z >= 0) ? (static_cast<int>(camera.Position.z) / CHUNK_DEPTH) : ((static_cast<int>(camera.Position.z) - CHUNK_DEPTH + 1) / CHUNK_DEPTH);

    // 2. Preload initial chunks (radius 4) and drain queues fully so frame 1 renders instantly
    world.UpdateStreaming(pChunkX, pChunkZ, 4, 12);
    world.ProcessQueues(99999.0, 99999.0);

    Frustum camFrustum;

    float lastTime = static_cast<float>(glfwGetTime());
    int nbFrames = 0;

    double startTime = glfwGetTime();
    long totalFramesRendered = 0;

    glfwSetWindowUserPointer(window, &camera);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Main Game Render Loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        totalFramesRendered++;

        // Sprint mechanic: Scale movement velocity if Left Control is held
        float velocityMultiplier = 2.0f;
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        {
            velocityMultiplier = 10.0f;
        }
        float currentDeltaTime = deltaTime * velocityMultiplier;

        // Keyboard movement inputs
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, currentDeltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, currentDeltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, currentDeltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, currentDeltaTime);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera.ProcessKeyboard(UP, currentDeltaTime);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            camera.ProcessKeyboard(DOWN, currentDeltaTime);

        processInput(window);

        // 3. Calculate live player chunk coordinates every frame
        int currentChunkX = (camera.Position.x >= 0) ? (static_cast<int>(camera.Position.x) / CHUNK_WIDTH) : ((static_cast<int>(camera.Position.x) - CHUNK_WIDTH + 1) / CHUNK_WIDTH);
        int currentChunkZ = (camera.Position.z >= 0) ? (static_cast<int>(camera.Position.z) / CHUNK_DEPTH) : ((static_cast<int>(camera.Position.z) - CHUNK_DEPTH + 1) / CHUNK_DEPTH);

        // 4. Update streaming with live coordinates and execute time-budgeted queue processing (1.5ms limit)
        world.UpdateStreaming(currentChunkX, currentChunkZ, 10, 12);
        world.ProcessQueues(1.5, 1.5);

        // Update live window title FPS counter every 1 second
        nbFrames++;
        if (currentFrame - lastTime >= 1.0f)
        {
            char fpsTitle[256];
            double fps = double(nbFrames) / (currentFrame - lastTime);
            snprintf(fpsTitle, sizeof(fpsTitle), "MineCraftPlusPlus | FPS: %.2f | Chunks: %zu | Quads: %zu",
                     fps, world.GetLoadedChunks().size(), world.GetTotalQuads());
            glfwSetWindowTitle(window, fpsTitle);

            nbFrames = 0;
            lastTime = currentFrame;
        }

        glClearColor(0.6f, 0.7f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        terrainAtlas.Bind(0);
        ourShader.use();

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 500.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // Update camera frustum planes for culling
        camFrustum.Update(projection * view);

        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // Render visible chunks within the camera's view frustum
        for (const auto &[coord, chunk] : world.GetLoadedChunks())
        {
            glm::vec3 minBounds(coord.first * CHUNK_WIDTH, 0.0f, coord.second * CHUNK_DEPTH);
            glm::vec3 maxBounds((coord.first + 1) * CHUNK_WIDTH, static_cast<float>(CHUNK_HEIGHT), (coord.second + 1) * CHUNK_DEPTH);

            // Frustum Culling: Skip rendering chunks outside the player's view
            if (!camFrustum.IsBoxVisible(minBounds, maxBounds))
            {
                continue;
            }

            glm::mat4 model = glm::translate(glm::mat4(1.0f), minBounds);
            ourShader.setMat4("model", model);
            chunk->Draw();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}