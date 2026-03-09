/*
 * Universidade Federal de Ouro Preto - UFOP
 * Departamento de Computação - DECOM
 * Disciplina: BCC327 - Computação Gráfica (2025.2)
 * Professor: Rafael Bonfim
 * Trabalho Prático: Visualizador de Árvores Arteriais (CCO)
 * Arquivo: ArterialTree.hpp
 * Autor: Mateus Honorato
 * Data: Fevereiro/2026
 * Descrição:
 * Declara as estruturas de dados para representar uma árvore arterial.
 */

#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

struct ArterialNode
{
    glm::vec3 position;
};

struct ArterialSegment
{
    int indexA;         // índice do nó inicial
    int indexB;         // índice do nó final
    float radius;       // raio (extraído de SCALARS)
    glm::vec3 midpoint; // ponto médio como "assinatura" do segmento
};

struct ArterialTree
{
    std::vector<ArterialNode> nodes;
    std::vector<ArterialSegment> segments;

    void normalize();
};
