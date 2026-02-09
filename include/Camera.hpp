#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
private:
    glm::vec3 target;
    float distance = 5.0f;
    float yaw = -90.0f;
    float pitch = 0.0f;
    float sensitivity = 0.5f;
    bool isDragging = false;
    float lastX = 400.0f, lastY = 300.0f;

public:
    Camera(glm::vec3 initialTarget = glm::vec3(0.0f));
    glm::mat4 getViewMatrix() const;
    glm::vec3 getPosition() const;
    void processMouseScroll(float yoffset);
    void processMouseButton(int button, int action, double xpos, double ypos);
    void processMouseMovement(double xpos, double ypos);
    void setDistance(float d);
};
