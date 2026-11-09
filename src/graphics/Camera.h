#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include<glm.hpp>
#include <gtc/matrix_transform.hpp>

// Defines directions for abstract keyboard input handling
enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 4.3f; // Standard walking speed approximation
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera
{
public:
    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // Euler Angles
    float Yaw;
    float Pitch;

    // Camera Options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // Constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

    // Constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

    // Returns the view matrix calculated using Euler Angles and LookAt
    glm::mat4 GetViewMatrix();

    // Handles keyboard inputs
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);

    // Handles mouse movement deltas for looking around
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

    // Handles mouse scroll wheel zoom
    void ProcessMouseScroll(float yoffset);

private:
    // Calculates the front vector from the updated Euler angles
    void updateCameraVectors();
};

#endif