/*
 * Universidade Federal de Ouro Preto - UFOP
 * Departamento de Computação - DECOM
 * Disciplina: BCC327 - Computação Gráfica (2025.2)
 * Professor: Rafael Bonfim
 * Trabalho Prático: Visualizador de Árvores Arteriais (CCO)
 * Arquivo: ClippingUtils.cpp
 * Autor: Mateus Honorato
 * Data: Fevereiro/2026
 * Descrição:
 * Implementa utilitários para recorte de segmentos (Liang-Barsky).
 */

#include "ClippingUtils.hpp"

// Implementação do Algoritmo de Liang-Barsky para recorte de segmentos de reta,
// conforme detalhado na Aula 17 da disciplina.

bool ClippingUtils::clipSegment(glm::vec3 &p0, glm::vec3 &p1, const glm::vec3 &boxMin, const glm::vec3 &boxMax)
{
    glm::vec3 p = p1 - p0;
    float t0 = 0.0f, t1 = 1.0f;
    for (int axis = 0; axis < 3; ++axis)
    {
        float min = boxMin[axis];
        float max = boxMax[axis];
        float d = p[axis];
        float q0 = p0[axis];
        // First inequality: min - q0 <= d * t
        if (d != 0.0f)
        {
            float tMin = (min - q0) / d;
            float tMax = (max - q0) / d;
            if (d > 0.0f)
            {
                if (tMin > t0)
                    t0 = tMin;
                if (tMax < t1)
                    t1 = tMax;
            }
            else
            {
                if (tMax > t0)
                    t0 = tMax;
                if (tMin < t1)
                    t1 = tMin;
            }
            if (t0 > t1)
                return false;
        }
        else
        {
            // Paralelo ao eixo: fora se não estiver dentro dos limites
            if (q0 < min || q0 > max)
                return false;
        }
    }
    glm::vec3 origP0 = p0;
    p1 = origP0 + t1 * p;
    p0 = origP0 + t0 * p;
    return true;
}