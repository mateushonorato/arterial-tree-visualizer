/*
 * Universidade Federal de Ouro Preto - UFOP
 * Departamento de Computação - DECOM
 * Disciplina: BCC327 - Computação Gráfica (2025.2)
 * Professor: Rafael Bonfim
 * Trabalho Prático: Visualizador de Árvores Arteriais (CCO)
 * Arquivo: SceneContext.hpp
 * Autor: Mateus Honorato
 * Data: Fevereiro/2026
 * Descrição:
 * Declara o contexto da cena responsável pela criação e desenho
 * da grade (grid) e do gizmo de orientação.
 */

#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>

class SceneContext
{
public:
    SceneContext();
    ~SceneContext();
    void init();
    void drawGrid(const glm::mat4 &view, const glm::mat4 &projection);
    void drawGizmo(const glm::mat4 &view, const glm::mat4 &projection, int screenWidth, int screenHeight);

private:
    GLuint gridVAO = 0, gridVBO = 0;
    GLuint gizmoVAO = 0, gizmoVBO = 0;
    GLuint shaderProgram = 0;
    int gridLineCount = 0;
    void createGridLines();
    void createGizmoAxes();
    GLuint compileShader(const char *vsrc, const char *fsrc);
};

// fim do arquivo
