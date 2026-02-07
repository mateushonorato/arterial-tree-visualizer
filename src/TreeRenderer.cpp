#include "TreeRenderer.hpp"
#include <vector>

TreeRenderer::~TreeRenderer() {
    if (EBO) glDeleteBuffers(1, &EBO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);
}

void TreeRenderer::init(const ArterialTree& tree) {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // VBO: positions
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, tree.nodes.size() * sizeof(ArterialNode), tree.nodes.data(), GL_STATIC_DRAW);

    // EBO: indices
    std::vector<unsigned int> indices;
    indices.reserve(tree.segments.size() * 2);
    for (const auto& seg : tree.segments) {
        indices.push_back(static_cast<unsigned int>(seg.indexA));
        indices.push_back(static_cast<unsigned int>(seg.indexB));
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Vertex attrib: position (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ArterialNode), (void*)0);

    glBindVertexArray(0);
    indexCount = indices.size();
}

void TreeRenderer::draw(Shader& shader, const glm::mat4& view, const glm::mat4& proj, const glm::mat4& model) {
    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", proj);
    shader.setMat4("model", model);
    shader.setVec3("color", glm::vec3(1.0f, 0.5f, 0.2f));
    glBindVertexArray(VAO);
    glDrawElements(GL_LINES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
