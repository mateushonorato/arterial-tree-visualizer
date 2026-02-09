// Includes must come first
#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

glm::vec3 Camera::getPosition() const {
    float yawRad = glm::radians(yaw);
    float pitchRad = glm::radians(pitch);
    float x = distance * cosf(pitchRad) * cosf(yawRad);
    float y = distance * sinf(pitchRad);
    float z = distance * cosf(pitchRad) * sinf(yawRad);
    return glm::vec3(x, y, z) + target;
}
#include <algorithm>
#include <cmath>

Camera::Camera(glm::vec3 initialTarget)
    : target(initialTarget) {}

glm::mat4 Camera::getViewMatrix() const {
    float yawRad = glm::radians(yaw);
    float pitchRad = glm::radians(pitch);
    float x = distance * cosf(pitchRad) * cosf(yawRad);
    float y = distance * sinf(pitchRad);
    float z = distance * cosf(pitchRad) * sinf(yawRad);
    glm::vec3 position = glm::vec3(x, y, z) + target;
    return glm::lookAt(position, target, glm::vec3(0, 1, 0));
}

void Camera::processMouseScroll(float yoffset) {
    if (yoffset > 0)
        distance *= 0.9f;
    else
        distance *= 1.1f;
    if (distance < 0.01f) distance = 0.01f;
    if (distance > 50.0f) distance = 50.0f;
}

void Camera::processMouseButton(int button, int action, double xpos, double ypos) {
    if (button == 0) { // GLFW_MOUSE_BUTTON_LEFT
        if (action == 1) { // GLFW_PRESS
            isDragging = true;
            lastX = static_cast<float>(xpos);
            lastY = static_cast<float>(ypos);
        } else if (action == 0) { // GLFW_RELEASE
            isDragging = false;
        }
    }
}

void Camera::processMouseMovement(double xpos, double ypos) {
    if (!isDragging) return;
    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos);
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);
    yaw += xoffset * sensitivity;
    pitch += yoffset * sensitivity;
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
}

void Camera::setDistance(float d) {
    distance = std::clamp(d, 0.1f, 50.0f);
}
