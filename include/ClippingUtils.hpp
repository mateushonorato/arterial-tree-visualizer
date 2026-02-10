/*
 * Universidade Federal de Ouro Preto - UFOP
 * Departamento de Computação - DECOM
 * Disciplina: BCC327 - Computação Gráfica (2025.2)
 * Professor: Rafael Bonfim
 * Trabalho Prático: Visualizador de Árvores Arteriais (CCO)
 * Arquivo: ClippingUtils.hpp
 * Autor: Mateus Honorato
 * Data: Fevereiro/2026
 * Descrição:
 * Declara utilitários para recorte de segmentos (algoritmo de Liang-Barsky).
 */

#pragma once
#include <glm/glm.hpp>

class ClippingUtils
{
public:
    // Implementação do algoritmo de Liang-Barsky para recorte de segmentos
    // Retorna true se o segmento (possivelmente recortado) está dentro da caixa
    static bool clipSegment(glm::vec3 &p0, glm::vec3 &p1, const glm::vec3 &boxMin, const glm::vec3 &boxMax);
};