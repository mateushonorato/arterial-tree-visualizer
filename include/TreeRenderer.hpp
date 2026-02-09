#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "VtkReader.hpp"
#include "shader.hpp"

class TreeRenderer {
private:
    unsigned int VAO = 0, VBO = 0, EBO = 0;
    size_t indexCount = 0;

public:
    TreeRenderer() = default;
    ~TreeRenderer();

    void init(const ArterialTree& tree, float radiusMultiplier = 1.0f, bool showSpheres = true);
    void draw(Shader& shader, const glm::mat4& view, const glm::mat4& proj, const glm::mat4& model);

private:
    void buildMeshes(const ArterialTree& tree, float radiusMultiplier, bool showSpheres);
    // Gera um cilindro conectando A e B
    void generateCylinder(const glm::vec3& a, const glm::vec3& b, float radiusA, float radiusB, const glm::vec3& colorA, const glm::vec3& colorB, std::vector<float>& vertices, std::vector<unsigned int>& indices);
    // Gera uma esfera (junta) no ponto center
    void generateSphere(const glm::vec3& center, float radius, const glm::vec3& color, std::vector<float>& vertices, std::vector<unsigned int>& indices);
    // Heatmap color helper
    glm::vec3 getHeatMapColor(float value, float minVal, float maxVal);
};