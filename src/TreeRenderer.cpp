#include "TreeRenderer.hpp"
#include <vector>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

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

// Helper para adicionar esfera (Junta)
void TreeRenderer::generateSphere(const glm::vec3& center, float radius, std::vector<float>& vertices, std::vector<unsigned int>& indices) {
    // Aumentamos a resolução para 24 para ficar mais suave no zoom
    const int X_SEGMENTS = 24; 
    const int Y_SEGMENTS = 24;
    
    unsigned int baseIdx = static_cast<unsigned int>(vertices.size() / 6);

    for (int y = 0; y <= Y_SEGMENTS; ++y) {
        for (int x = 0; x <= X_SEGMENTS; ++x) {
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
            
            float xPos = std::cos(xSegment * 2.0f * (float)M_PI) * std::sin(ySegment * (float)M_PI);
            float yPos = std::cos(ySegment * (float)M_PI);
            float zPos = std::sin(xSegment * 2.0f * (float)M_PI) * std::sin(ySegment * (float)M_PI);

            glm::vec3 normal = glm::vec3(xPos, yPos, zPos);
            glm::vec3 pos = center + (normal * radius);

            // Vertex: Pos(3) + Normal(3)
            vertices.insert(vertices.end(), {pos.x, pos.y, pos.z, normal.x, normal.y, normal.z});
        }
    }

    for (int y = 0; y < Y_SEGMENTS; ++y) {
        for (int x = 0; x < X_SEGMENTS; ++x) {
            unsigned int k1 = baseIdx + y * (X_SEGMENTS + 1) + x;
            unsigned int k2 = baseIdx + y * (X_SEGMENTS + 1) + x + 1;
            unsigned int k3 = baseIdx + (y + 1) * (X_SEGMENTS + 1) + x;
            unsigned int k4 = baseIdx + (y + 1) * (X_SEGMENTS + 1) + x + 1;

            indices.push_back(k1);
            indices.push_back(k3);
            indices.push_back(k2);

            indices.push_back(k2);
            indices.push_back(k3);
            indices.push_back(k4);
        }
    }
}

// Helper para adicionar cilindro (Ramo)
void TreeRenderer::generateCylinder(const glm::vec3& a, const glm::vec3& b, float radius, std::vector<float>& vertices, std::vector<unsigned int>& indices) {
    const int segments = 24; // Resolução maior para casar com a esfera
    
    glm::vec3 axis = glm::normalize(b - a);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    if (glm::abs(glm::dot(axis, up)) > 0.99f) up = glm::vec3(1.0f, 0.0f, 0.0f);
    
    glm::vec3 side = glm::normalize(glm::cross(axis, up));
    glm::vec3 ortho = glm::normalize(glm::cross(axis, side));

    unsigned int baseIdx = static_cast<unsigned int>(vertices.size() / 6);

    // Gera vértices
    for (int i = 0; i <= segments; ++i) { // Note o <= para fechar o loop de textura/normal corretamente
        float theta = (float)i / (float)segments * 2.0f * (float)M_PI;
        float x = cosf(theta);
        float y = sinf(theta);
        
        glm::vec3 offset = side * x * radius + ortho * y * radius;
        glm::vec3 normal = glm::normalize(offset); 

        // Base (Ponto A)
        glm::vec3 p1 = a + offset;
        vertices.insert(vertices.end(), {p1.x, p1.y, p1.z, normal.x, normal.y, normal.z});
        
        // Topo (Ponto B)
        glm::vec3 p2 = b + offset;
        vertices.insert(vertices.end(), {p2.x, p2.y, p2.z, normal.x, normal.y, normal.z});
    }

    // Gera índices
    for (int i = 0; i < segments; ++i) {
        unsigned int current = baseIdx + i * 2;
        unsigned int next = baseIdx + (i + 1) * 2;

        unsigned int b0 = current;      // Base atual
        unsigned int t0 = current + 1;  // Topo atual
        unsigned int b1 = next;         // Base próxima
        unsigned int t1 = next + 1;     // Topo próximo

        // Triângulo 1
        indices.push_back(b0);
        indices.push_back(t0);
        indices.push_back(b1);

        // Triângulo 2
        indices.push_back(t0);
        indices.push_back(t1);
        indices.push_back(b1);
    }
}

void TreeRenderer::buildMeshes(const ArterialTree& tree, float radiusMultiplier) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    // Reserva memória para performance (opcional)
    // vertices.reserve(tree.segments.size() * 1000);

    for (const auto& seg : tree.segments) {
        glm::vec3 a = tree.nodes[seg.indexA].position;
        glm::vec3 b = tree.nodes[seg.indexB].position;
        
        float radius = glm::max(seg.radius * radiusMultiplier, 0.002f);
        
        // 1. Gera o Tubo
        generateCylinder(a, b, radius, vertices, indices);
        
        // 2. Gera as Juntas (Esferas)
        float jointRadius = radius; 
        
        generateSphere(a, jointRadius, vertices, indices);
        generateSphere(b, jointRadius, vertices, indices);
    }

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Layout 0: Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); 
    
    // Layout 1: Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
    indexCount = indices.size();
}

void TreeRenderer::draw(Shader& shader, const glm::mat4& view, const glm::mat4& proj, const glm::mat4& model) {
    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", proj);
    shader.setMat4("model", model);
    
    // Cor temporária (Laranja) até implementarmos o Gradiente
    // shader.setVec3("color", glm::vec3(1.0f, 0.5f, 0.2f)); 
    
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}