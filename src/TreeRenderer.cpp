#include "TreeRenderer.hpp"
#include <vector>

TreeRenderer::~TreeRenderer() {
    if (EBO) glDeleteBuffers(1, &EBO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);
}

void TreeRenderer::init(const ArterialTree& tree, float radiusMultiplier) {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    buildMeshes(tree, radiusMultiplier);
}

void TreeRenderer::buildMeshes(const ArterialTree& tree, float radiusMultiplier) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    unsigned int baseIdx = 0;
    const int segments = 16;
    for (const auto& seg : tree.segments) {
        glm::vec3 a = tree.nodes[seg.indexA].position;
        glm::vec3 b = tree.nodes[seg.indexB].position;
        float radius = glm::max(seg.radius * radiusMultiplier, 0.002f);
        generateCylinder(a, b, radius, vertices, indices);
        baseIdx += segments * 2;
    }
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // position
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // normal
    glBindVertexArray(0);
    indexCount = indices.size();
}

void TreeRenderer::generateCylinder(const glm::vec3& a, const glm::vec3& b, float radius, std::vector<float>& vertices, std::vector<unsigned int>& indices) {
    const int segments = 16;
    glm::vec3 axis = glm::normalize(b - a);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    if (glm::abs(glm::dot(axis, up)) > 0.99f) up = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 side = glm::normalize(glm::cross(axis, up));
    glm::vec3 ortho = glm::normalize(glm::cross(axis, side));
    int baseIdx = vertices.size() / 6;
    for (int i = 0; i < segments; ++i) {
        float theta = (float)i / segments * 2.0f * 3.1415926f;
        float x = cosf(theta);
        float y = sinf(theta);
        glm::vec3 offset = side * x * radius + ortho * y * radius;
        glm::vec3 p1 = a + offset;
        glm::vec3 p2 = b + offset;
        glm::vec3 normal = glm::normalize(offset);
        // Vertex: position + normal
        vertices.insert(vertices.end(), {p1.x, p1.y, p1.z, normal.x, normal.y, normal.z});
        vertices.insert(vertices.end(), {p2.x, p2.y, p2.z, normal.x, normal.y, normal.z});
    }
    for (int i = 0; i < segments; ++i) {
        int i1 = baseIdx + i * 2;
        int i2 = baseIdx + ((i + 1) % segments) * 2;
        int i3 = i1 + 1;
        int i4 = i2 + 1;
        // Triangle 1
        indices.push_back(i1);
        indices.push_back(i2);
        indices.push_back(i3);
        // Triangle 2
        indices.push_back(i3);
        indices.push_back(i2);
        indices.push_back(i4);
    }
}

void TreeRenderer::draw(Shader& shader, const glm::mat4& view, const glm::mat4& proj, const glm::mat4& model) {
    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", proj);
    shader.setMat4("model", model);
    shader.setVec3("color", glm::vec3(1.0f, 0.5f, 0.2f));
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
