#include "TreeRenderer.hpp"
#include <vector>
#include <cmath>
#include <algorithm>
#include <limits>
#include <map>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

TreeRenderer::~TreeRenderer() {
    if (EBO) glDeleteBuffers(1, &EBO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);
}

// Helper: Gradiente de Cor (Mapa de Calor)
glm::vec3 TreeRenderer::getHeatMapColor(float value, float minVal, float maxVal) {
    if (maxVal - minVal < 0.00001f) return glm::vec3(0.0f, 1.0f, 0.0f); 

    float t = (value - minVal) / (maxVal - minVal);
    t = std::clamp(t, 0.0f, 1.0f);

    // Gradiente: Azul (Fino) -> Verde -> Vermelho (Grosso)
    if (t < 0.5f) {
        return glm::mix(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), t * 2.0f);
    } else {
        return glm::mix(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), (t - 0.5f) * 2.0f);
    }
}

void TreeRenderer::init(const ArterialTree& tree, float radiusMultiplier, bool showSpheres) {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    buildMeshes(tree, radiusMultiplier, showSpheres);
}

void TreeRenderer::generateSphere(const glm::vec3& center, float radius, const glm::vec3& color, std::vector<float>& vertices, std::vector<unsigned int>& indices) {
    // Alta resolução (32) para minimizar quinas visíveis
    const int X_SEGMENTS = 32; 
    const int Y_SEGMENTS = 32;
    
    unsigned int baseIdx = static_cast<unsigned int>(vertices.size() / 9);

    for (int y = 0; y <= Y_SEGMENTS; ++y) {
        for (int x = 0; x <= X_SEGMENTS; ++x) {
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
            
            float xPos = std::cos(xSegment * 2.0f * (float)M_PI) * std::sin(ySegment * (float)M_PI);
            float yPos = std::cos(ySegment * (float)M_PI);
            float zPos = std::sin(xSegment * 2.0f * (float)M_PI) * std::sin(ySegment * (float)M_PI);

            glm::vec3 normal = glm::vec3(xPos, yPos, zPos);
            glm::vec3 pos = center + (normal * radius);

            vertices.insert(vertices.end(), {
                pos.x, pos.y, pos.z, 
                normal.x, normal.y, normal.z,
                color.r, color.g, color.b
            });
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

void TreeRenderer::generateCylinder(const glm::vec3& a, const glm::vec3& b, float radiusA, float radiusB, const glm::vec3& colorA, const glm::vec3& colorB, std::vector<float>& vertices, std::vector<unsigned int>& indices) {
    // Alta resolução (32) para casar perfeitamente com a esfera
    const int segments = 32; 
    
    glm::vec3 axis = glm::normalize(b - a);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    if (glm::abs(glm::dot(axis, up)) > 0.99f) up = glm::vec3(1.0f, 0.0f, 0.0f);
    
    glm::vec3 side = glm::normalize(glm::cross(axis, up));
    glm::vec3 ortho = glm::normalize(glm::cross(axis, side));

    unsigned int baseIdx = static_cast<unsigned int>(vertices.size() / 9);

    for (int i = 0; i <= segments; ++i) { 
        float theta = (float)i / (float)segments * 2.0f * (float)M_PI;
        float x = cosf(theta);
        float y = sinf(theta);
        
        glm::vec3 dirVec = side * x + ortho * y; 
        glm::vec3 normal = glm::normalize(dirVec); 

        // Base (Ponto A)
        glm::vec3 p1 = a + (dirVec * radiusA);
        vertices.insert(vertices.end(), {
            p1.x, p1.y, p1.z, 
            normal.x, normal.y, normal.z,
            colorA.r, colorA.g, colorA.b
        });
        
        // Topo (Ponto B)
        glm::vec3 p2 = b + (dirVec * radiusB);
        vertices.insert(vertices.end(), {
            p2.x, p2.y, p2.z, 
            normal.x, normal.y, normal.z,
            colorB.r, colorB.g, colorB.b
        });
    }

    for (int i = 0; i < segments; ++i) {
        unsigned int current = baseIdx + i * 2;
        unsigned int next = baseIdx + (i + 1) * 2;
        indices.push_back(current);
        indices.push_back(current + 1);
        indices.push_back(next);
        indices.push_back(current + 1);
        indices.push_back(next + 1);
        indices.push_back(next);
    }
}

void TreeRenderer::buildMeshes(const ArterialTree& tree, float radiusMultiplier, bool showSpheres) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    std::vector<float> nodeMaxRadii(tree.nodes.size(), 0.0f);
    std::vector<int> nodeCounts(tree.nodes.size(), 0); 

    float minRadius = std::numeric_limits<float>::max();
    float maxRadius = std::numeric_limits<float>::lowest();

    // 1. Estatística: Raio Máximo por Nó
    for (const auto& seg : tree.segments) {
        if (seg.radius < minRadius) minRadius = seg.radius;
        if (seg.radius > maxRadius) maxRadius = seg.radius;

        nodeMaxRadii[seg.indexA] = std::max(nodeMaxRadii[seg.indexA], seg.radius);
        nodeCounts[seg.indexA]++;
        nodeMaxRadii[seg.indexB] = std::max(nodeMaxRadii[seg.indexB], seg.radius);
        nodeCounts[seg.indexB]++;
    }

    // 2. Geometria: CILINDROS (Ramos)
    for (const auto& seg : tree.segments) {
        glm::vec3 a = tree.nodes[seg.indexA].position;
        glm::vec3 b = tree.nodes[seg.indexB].position;
        
        // Raio Exato (sem reductionFactor)
        float radiusA = glm::max(nodeMaxRadii[seg.indexA] * radiusMultiplier, 0.002f);
        float radiusB = glm::max(nodeMaxRadii[seg.indexB] * radiusMultiplier, 0.002f);
        
        glm::vec3 colorA = getHeatMapColor(nodeMaxRadii[seg.indexA], minRadius, maxRadius);
        glm::vec3 colorB = getHeatMapColor(nodeMaxRadii[seg.indexB], minRadius, maxRadius);

        generateCylinder(a, b, radiusA, radiusB, colorA, colorB, vertices, indices);
    }
        
    // 3. Geometria: ESFERAS (Juntas)
    // Loop separado para evitar overdraw
    if (showSpheres) {
        for (size_t i = 0; i < tree.nodes.size(); ++i) {
            if (nodeCounts[i] > 1) { // Apenas se for junção
                glm::vec3 center = tree.nodes[i].position;
                
                // Raio Exato (sem aumento)
                float radius = glm::max(nodeMaxRadii[i] * radiusMultiplier, 0.002f);
                glm::vec3 color = getHeatMapColor(nodeMaxRadii[i], minRadius, maxRadius);

                generateSphere(center, radius, color, vertices, indices);
            }
        }
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    GLsizei stride = 9 * sizeof(float);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0); 
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));

    glBindVertexArray(0);
    indexCount = indices.size();
}

void TreeRenderer::draw(Shader& shader, const glm::mat4& view, const glm::mat4& proj, const glm::mat4& model) {
    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", proj);
    shader.setMat4("model", model);
    
    glBindVertexArray(VAO);

    // CORREÇÃO: Polygon Offset
    // Empurra ligeiramente os fragmentos para o fundo do Z-Buffer.
    // Isso ajuda a resolver conflitos quando geometrias compartilham o mesmo espaço.
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f); 

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, 0);

    glDisable(GL_POLYGON_OFFSET_FILL);
    glBindVertexArray(0);
}