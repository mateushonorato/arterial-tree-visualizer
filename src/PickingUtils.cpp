/*
 * Universidade Federal de Ouro Preto - UFOP
 * Departamento de Computação - DECOM
 * Disciplina: BCC327 - Computação Gráfica (2025.2)
 * Professor: Rafael Bonfim
 * Trabalho Prático: Visualizador de Árvores Arteriais (CCO)
 * Arquivo: PickingUtils.cpp
 * Autor: Mateus Honorato
 * Data: Fevereiro/2026
 * Descrição:
 * Implementa utilitários de picking por raio (unProject + interseção).
 *
 * Créditos:
 * Baseado em documentação técnica da biblioteca GLM (unProject) e teoria
 * de Ray Casting.
 */

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp> // Necessário para unProject
#include "PickingUtils.hpp"

// Implementação de Ray Casting utilizando a inversão das matrizes de Projeção
// e View (glm::unProject).

namespace PickingUtils
{

    void getRayFromMouse(
        double mouseX, double mouseY,
        int screenW, int screenH,
        const glm::mat4 &view,
        const glm::mat4 &projection,
        glm::vec3 &outOrigin,
        glm::vec3 &outDir)
    {
        // O Y do mouse vem invertido (topo-esquerda), OpenGL usa baixo-esquerda
        float winY = (float)screenH - (float)mouseY;

        // Define o Viewport
        glm::vec4 viewport(0.0f, 0.0f, (float)screenW, (float)screenH);

        // Converte coordenadas de tela (2D) para espaço do mundo (3D).
        // `glm::unProject` aplica a inversa das transformações de
        // Projeção e View para reconstruir as posições 3D correspondentes
        // aos pontos no plano próximo (z=0) e no plano distante (z=1).
        // A partir desses dois pontos é obtido o raio de picking no mundo:
        // `outOrigin` no Near Plane e `outDir` como direção normalizada
        // em direção ao Far Plane.
        glm::vec3 nearPoint = glm::unProject(
            glm::vec3(mouseX, winY, 0.0f),
            view,
            projection,
            viewport);

        glm::vec3 farPoint = glm::unProject(
            glm::vec3(mouseX, winY, 1.0f),
            view,
            projection,
            viewport);

        outOrigin = nearPoint;
        outDir = glm::normalize(farPoint - nearPoint);
    }

    // Calcula a interseção entre um raio e um segmento cilíndrico (segmento
    // com raio). A técnica busca os pontos mais próximos entre a linha do
    // raio e a linha do segmento, resolvendo para os parâmetros escalar do
    // raio (sc) e do segmento (tc). Em seguida verifica-se a distância
    // mínima entre esses pontos e compara com o raio do segmento.
    bool rayIntersectsSegment(
        glm::vec3 rayOrigin,
        glm::vec3 rayDir,
        glm::vec3 segA,
        glm::vec3 segB,
        float segRadius,
        float &outDist)
    {
        // Implementação: resolve o sistema para minimizar a distância entre
        // dois segmentos de reta (um é o raio parametrizado por sc, outro
        // é o segmento parametrizado por tc), cuidando do caso degenerado
        // em que o segmento tem comprimento quase zero.
        glm::vec3 v = segB - segA;
        glm::vec3 w0 = rayOrigin - segA;
        float lenSq = glm::dot(v, v);
        if (lenSq < 1e-6f)
            return false;

        float a = glm::dot(rayDir, rayDir);
        float b = glm::dot(rayDir, v);
        float c = lenSq;
        float d = glm::dot(rayDir, w0);
        float e = glm::dot(v, w0);
        float denom = a * c - b * b;
        float sc, tc;

        if (denom < 1e-6f)
        {
            sc = 0.0f;
            tc = (b > c ? d / b : e / c);
        }
        else
        {
            sc = (b * e - c * d) / denom;
            tc = (a * e - b * d) / denom;
        }

        if (sc < 0.0f)
            return false;

        float tSeg = glm::clamp(tc, 0.0f, 1.0f);
        glm::vec3 ptSeg = segA + tSeg * v;
        glm::vec3 ptRay = rayOrigin + sc * rayDir;

        float dist = glm::length(ptRay - ptSeg);
        if (dist <= segRadius)
        {
            outDist = glm::length(ptRay - rayOrigin);
            return true;
        }
        return false;
    }
}