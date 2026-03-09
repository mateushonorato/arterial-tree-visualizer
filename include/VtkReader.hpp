/*
 * Universidade Federal de Ouro Preto - UFOP
 * Departamento de Computação - DECOM
 * Disciplina: BCC327 - Computação Gráfica (2025.2)
 * Professor: Rafael Bonfim
 * Trabalho Prático: Visualizador de Árvores Arteriais (CCO)
 * Arquivo: VtkReader.hpp
 * Autor: Mateus Honorato
 * Data: Fevereiro/2026
 * Descrição:
 * Declara um leitor simples para arquivos VTK representando a árvore
 * arterial (nós e segmentos).
 */

#pragma once

#include "ArterialTree.hpp"
#include <string>

class VtkReader
{
public:
    static bool load(const std::string &filepath, ArterialTree &outTree);
};
