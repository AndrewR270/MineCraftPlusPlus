#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include "graphics/Camera.h"
#include "graphics/Shader.h"
#include "graphics/Mesh.h"
#include "graphics/Texture.h"
#include "world/BlockRegistry.h"
#include "world/Chunk.h"
#include <iostream>
#include <vector>

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    Camera *camera = static_cast<Camera *>(glfwGetWindowUserPointer(window));
    if (camera) {
        camera->ProcessMouseMovement(xoffset, yoffset);
    }
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }


    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Minecraft Beta Engine - C++", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Disable V-Sync for unthrottled benchmarking (set to 1 to lock to monitor refresh)
    glfwSwapInterval(0);
    
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader ourShader("shaders/vertex.vs", "shaders/fragment.fs");

    // 1. Generate optimized Chunk mesh with Face Culling
    unsigned int worldSeed = static_cast<unsigned int>(time(nullptr));
    srand(worldSeed);

    Chunk worldChunk(0, 0, worldSeed);
    std::vector<float> chunkVertices = worldChunk.GenerateMesh();
    Mesh chunkMesh(chunkVertices.data(), chunkVertices.size() * sizeof(float), chunkVertices.size() / 7);

    // Load Atlas Texture
    Texture terrainAtlas("assets/textures/textures.png");
    ourShader.use();
    ourShader.setInt("texture1", 0);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    Camera camera(glm::vec3(8.0f, 85.0f, 25.0f)); // Positioned above terrain looking down

    // Performance tracking variables
    float lastTime = static_cast<float>(glfwGetTime());
    int nbFrames = 0;

    double startTime = glfwGetTime();
    long totalFramesRendered = 0;

    glfwSetWindowUserPointer(window, &camera);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        totalFramesRendered++;

        // Update live window title FPS every 1 second
        nbFrames++;
        if (currentFrame - lastTime >= 1.0f) {
            char fpsTitle[256];
            double fps = double(nbFrames) / (currentFrame - lastTime);
            snprintf(fpsTitle, sizeof(fpsTitle), "Minecraft Beta Engine - C++ | FPS: %.2f | Quads: %zu", 
                     fps, chunkVertices.size() / (7 * 6));
            glfwSetWindowTitle(window, fpsTitle);
            
            nbFrames = 0;
            lastTime = currentFrame;
        }

        // SPRINT MECHANIC: Check if Left Control is pressed to scale movement speed
        float velocityMultiplier = 1.0f;
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            velocityMultiplier = 5.0f; // Adjust this factor to sprint faster or slower
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

        glClearColor(0.6f, 0.7f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        terrainAtlas.Bind(0);
        ourShader.use();

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);

        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("view", view);

        // Render entire chunk in a single draw call (Identity model matrix)
        glm::mat4 model = glm::mat4(1.0f);
        ourShader.setMat4("model", model);
        chunkMesh.Draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Print summary stats on window exit
    double endTime = glfwGetTime();
    double totalSessionTime = endTime - startTime;
    double lifetimeAvgFps = (totalSessionTime > 0.0) ? (totalFramesRendered / totalSessionTime) : 0.0;

    std::cout << "\n=========================================" << std::endl;
    std::cout << "   ENGINE BENCHMARK PERFORMANCE LOG     " << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "Total Frames Rendered: " << totalFramesRendered << std::endl;
    std::cout << "Session Duration:      " << totalSessionTime << " s" << std::endl;
    std::cout << "Lifetime Average FPS:  " << lifetimeAvgFps << " FPS" << std::endl;
    std::cout << "=========================================\n" << std::endl;

    glfwTerminate();
    return 0;
}