#ifndef SCENE_CONTEXT_HPP
#define SCENE_CONTEXT_HPP

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <vector>
#include <string>

class SceneContext {
public:
    SceneContext();
    ~SceneContext();
    void init();
    void drawGrid(const glm::mat4& view, const glm::mat4& projection);
    void drawGizmo(const glm::mat4& view, const glm::mat4& projection, int screenWidth, int screenHeight);
private:
    GLuint gridVAO = 0, gridVBO = 0;
    GLuint gizmoVAO = 0, gizmoVBO = 0;
    GLuint shaderProgram = 0;
    int gridLineCount = 0;
    void createGridLines();
    void createGizmoAxes();
    GLuint compileShader(const char* vsrc, const char* fsrc);
};

#endif // SCENE_CONTEXT_HPP
