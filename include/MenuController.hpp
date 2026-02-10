/*
 * Universidade Federal de Ouro Preto - UFOP
 * Departamento de Computação - DECOM
 * Disciplina: BCC327 - Computação Gráfica (2025.2)
 * Professor: Rafael Bonfim
 * Trabalho Prático: Visualizador de Árvores Arteriais (CCO)
 * Arquivo: MenuController.hpp
 * Autor: Mateus Honorato
 * Data: Fevereiro/2026
 * Descrição:
 * Declara o controlador do menu de interface (renderização via ImGui).
 */

#pragma once

#include "AnimationController.hpp"

class MenuController
{
public:
    void render(AnimationController &animCtrl, ArterialTree &tree, TreeRenderer &renderer, bool hideMainPanel = false);
};