#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include "graphics/Camera.h"
#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/Frustum.h"
#include "world/BlockRegistry.h"
#include "world/World.h"
#include <iostream>

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

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
    float yoffset = lastY - ypos;
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

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Minecraft Beta Engine - C++", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Disable V-Sync for unthrottled benchmarking (set to 1 to lock to monitor refresh)
    glfwSwapInterval(0);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader ourShader("shaders/vertex.vs", "shaders/fragment.fs");

    // Initialize World with seed
    unsigned int worldSeed = static_cast<unsigned int>(time(nullptr));
    srand(worldSeed);

    World world(worldSeed);

    // Load Atlas Texture
    Texture terrainAtlas("assets/textures/textures.png");
    ourShader.use();
    ourShader.setInt("texture1", 0);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    Camera camera(glm::vec3(8.0f, 85.0f, 25.0f)); // Positioned above terrain looking down

    // 1. Calculate initial player chunk coordinates
    int pChunkX = (camera.Position.x >= 0) ? (static_cast<int>(camera.Position.x) / CHUNK_WIDTH) : ((static_cast<int>(camera.Position.x) - CHUNK_WIDTH + 1) / CHUNK_WIDTH);
    int pChunkZ = (camera.Position.z >= 0) ? (static_cast<int>(camera.Position.z) / CHUNK_DEPTH) : ((static_cast<int>(camera.Position.z) - CHUNK_DEPTH + 1) / CHUNK_DEPTH);

    // 2. Preload the initial render distance area (e.g. radius 16) and flush the mesh queue instantly on startup
    world.UpdateStreaming(pChunkX, pChunkZ, 16);
    world.ProcessMeshQueue(999999); // Drain the queue fully so frame 1 has everything built

    Frustum camFrustum;

    // Performance tracking variables
    float lastTime = static_cast<float>(glfwGetTime());
    int nbFrames = 0;

    double startTime = glfwGetTime();
    long totalFramesRendered = 0;

    glfwSetWindowUserPointer(window, &camera);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        totalFramesRendered++;

        // SPRINT MECHANIC: Check if Left Control is pressed to scale movement speed
        float velocityMultiplier = 1.0f;
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        {
            velocityMultiplier = 10.0f;
        }
        float currentDeltaTime = deltaTime * velocityMultiplier;

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

        // Calculate player chunk coordinates from camera position
        int pChunkX = (camera.Position.x >= 0) ? (static_cast<int>(camera.Position.x) / CHUNK_WIDTH) : ((static_cast<int>(camera.Position.x) - CHUNK_WIDTH + 1) / CHUNK_WIDTH);
        int pChunkZ = (camera.Position.z >= 0) ? (static_cast<int>(camera.Position.z) / CHUNK_DEPTH) : ((static_cast<int>(camera.Position.z) - CHUNK_DEPTH + 1) / CHUNK_DEPTH);

        // Update dynamic chunk loading/unloading (e.g. render distance 16 chunks)
        world.UpdateStreaming(pChunkX, pChunkZ, 16);

        // Process at most 1 chunk mesh upload per frame to maintain butter-smooth framerates
        world.ProcessMeshQueue(1);

        // Update live window title FPS every 1 second
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

        camFrustum.Update(projection * view);

        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // Render visible chunks with their world-space offsets
        for (const auto &[coord, chunk] : world.GetLoadedChunks())
        {
            glm::vec3 minBounds(coord.first * CHUNK_WIDTH, 0.0f, coord.second * CHUNK_DEPTH);
            glm::vec3 maxBounds((coord.first + 1) * CHUNK_WIDTH, static_cast<float>(CHUNK_HEIGHT), (coord.second + 1) * CHUNK_DEPTH);

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

    double endTime = glfwGetTime();
    double totalSessionTime = endTime - startTime;
    double lifetimeAvgFps = (totalSessionTime > 0.0) ? (totalFramesRendered / totalSessionTime) : 0.0;

    std::cout << "\n=========================================" << std::endl;
    std::cout << "   ENGINE BENCHMARK PERFORMANCE LOG     " << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "Total Frames Rendered: " << totalFramesRendered << std::endl;
    std::cout << "Session Duration:      " << totalSessionTime << " s" << std::endl;
    std::cout << "Lifetime Average FPS:  " << lifetimeAvgFps << " FPS" << std::endl;
    std::cout << "=========================================\n"
              << std::endl;

    glfwTerminate();
    return 0;
}