#version 330 core
/*
 * Universidade Federal de Ouro Preto - UFOP
 * Departamento de Computação - DECOM
 * Disciplina: BCC327 - Computação Gráfica (2025.2)
 * Professor: Rafael Bonfim
 * Trabalho Prático: Visualizador de Árvores Arteriais (CCO)
 * Arquivo: line_vertex.glsl
 * Autor: Mateus Honorato
 * Data: Fevereiro/2026
 * Descrição:
 * Vertex shader simplificado para renderização de linhas (wireframe).
 */

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec3 aColor;
layout (location = 3) in int aSegmentID;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Color;
flat out int vSegmentID;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    Color = aColor;
    vSegmentID = aSegmentID;
}
