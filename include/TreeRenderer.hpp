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

    void init(const ArterialTree& tree);
    void draw(Shader& shader, const glm::mat4& view, const glm::mat4& proj, const glm::mat4& model);
};
