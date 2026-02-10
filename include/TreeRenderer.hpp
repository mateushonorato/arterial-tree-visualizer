#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "VtkReader.hpp"
#include "Shader.hpp"

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec3 color;
    int segmentID;
};

struct WireframeRenderBuffers {
    GLuint vao = 0;
    GLuint vbo = 0;
    size_t vertexCount = 0;
};

class TreeRenderer {
private:
    unsigned int VAO = 0, VBO = 0, EBO = 0;
    size_t indexCount = 0;

    // Wireframe mode buffers
    WireframeRenderBuffers wireframeBuf;

public:
    TreeRenderer() = default;
    ~TreeRenderer();

    void init(const ArterialTree& tree, float radiusMultiplier = 1.0f, bool showSpheres = true,
              bool clipEnabled = false, glm::vec3 clipMin = glm::vec3(-1000.0f), glm::vec3 clipMax = glm::vec3(1000.0f));
    void draw(Shader& shader, const glm::mat4& view, const glm::mat4& proj, const glm::mat4& model, int selectedSegmentID = -1);

    // Wireframe mode
    void initWireframe(const std::vector<ArterialNode>& nodes, const std::vector<ArterialSegment>& segments,
                      bool clipEnabled, glm::vec3 clipMin, glm::vec3 clipMax);
    void drawWireframe(Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model, float width, int selectedSegmentID = -1);

private:
    void buildMeshes(const ArterialTree& tree, float radiusMultiplier, bool showSpheres,
                    bool clipEnabled, glm::vec3 clipMin, glm::vec3 clipMax);
    void generateCylinder(const glm::vec3& a, const glm::vec3& b, float radiusA, float radiusB, const glm::vec3& colorA, const glm::vec3& colorB, int segmentID, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices);
    void generateSphere(const glm::vec3& center, float radius, const glm::vec3& color, int segmentID, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices);
    glm::vec3 getHeatMapColor(float value, float minVal, float maxVal);
};