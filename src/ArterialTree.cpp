/*
 * Universidade Federal de Ouro Preto - UFOP
 * Departamento de Computação - DECOM
 * Disciplina: BCC327 - Computação Gráfica (2025.2)
 * Professor: Rafael Bonfim
 * Trabalho Prático: Visualizador de Árvores Arteriais (CCO)
 * Arquivo: ArterialTree.cpp
 * Autor: Mateus Honorato
 * Data: Fevereiro/2026
 * Descrição:
 * Implementa a normalização da árvore arterial.
 */

#include "ArterialTree.hpp"
#include <limits>
#include <algorithm>

void ArterialTree::normalize()
{
    if (nodes.empty())
        return;
    // 1. Computar caixa delimitadora
    glm::vec3 minPos(std::numeric_limits<float>::max());
    glm::vec3 maxPos(-std::numeric_limits<float>::max());
    for (const auto &node : nodes)
    {
        minPos = glm::min(minPos, node.position);
        maxPos = glm::max(maxPos, node.position);
    }
    // 2. Computar dimensão máxima
    float maxDim = glm::max(glm::max(maxPos.x - minPos.x, maxPos.y - minPos.y), maxPos.z - minPos.z);
    if (maxDim < 1e-6f)
        maxDim = 1.0f;
    // 3. Fator de escala para caber em volume canônico
    float scaleFactor = 2.0f / maxDim;
    // 4. Centralizar na raiz (sem jitter)
    glm::vec3 center = nodes[0].position;
    for (auto &node : nodes)
        node.position = (node.position - center) * scaleFactor;
    // 5. Encontrar maior raio
    float maxRadius = 0.0f;
    for (const auto &seg : segments)
        maxRadius = std::max(maxRadius, seg.radius);
    float maxRadiusScaled = maxRadius * scaleFactor;
    float fixFactor = 1.0f;
    // 6. Heurística: se maxRadiusScaled for grande, reduzir raios
    if (maxRadiusScaled > 0.2f)
    {
        fixFactor = 0.05f / maxRadiusScaled;
    }
    // 7. Aplicar escala e correção aos raios
    for (auto &seg : segments)
        seg.radius = seg.radius * scaleFactor * fixFactor;
}
