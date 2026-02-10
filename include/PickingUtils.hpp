/*
 * Universidade Federal de Ouro Preto - UFOP
 * Departamento de Computação - DECOM
 * Disciplina: BCC327 - Computação Gráfica (2025.2)
 * Professor: Rafael Bonfim
 * Trabalho Prático: Visualizador de Árvores Arteriais (CCO)
 * Arquivo: PickingUtils.hpp
 * Autor: Mateus Honorato
 * Data: Fevereiro/2026
 * Descrição:
 * Declara funções utilitárias para seleção por raio (picking).
 */

#pragma once

#include <glm/glm.hpp>

namespace PickingUtils
{
    // Calcula origem e direção do raio (perspectiva e ortográfica)
    void getRayFromMouse(
        double mouseX, double mouseY,
        int screenW, int screenH,
        const glm::mat4 &view,
        const glm::mat4 &projection,
        glm::vec3 &outOrigin,
        glm::vec3 &outDir);

    bool rayIntersectsSegment(
        glm::vec3 rayOrigin,
        glm::vec3 rayDir,
        glm::vec3 segA,
        glm::vec3 segB,
        float segRadius,
        float &outDist);
}