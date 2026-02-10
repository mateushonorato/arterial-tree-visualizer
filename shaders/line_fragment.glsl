#version 330 core
/*
 * Universidade Federal de Ouro Preto - UFOP
 * Departamento de Computação - DECOM
 * Disciplina: BCC327 - Computação Gráfica (2025.2)
 * Professor: Rafael Bonfim
 * Trabalho Prático: Visualizador de Árvores Arteriais (CCO)
 * Arquivo: line_fragment.glsl
 * Autor: Mateus Honorato
 * Data: Fevereiro/2026
 * Descrição:
 * Fragment shader simplificado para renderização de linhas (wireframe) com
 * suporte a destaque de segmento selecionado.
 */

in vec3 Color;
flat in int vSegmentID;

uniform int selectedSegmentID;
uniform float alpha;

out vec4 FragColor;

void main()
{
    vec4 outColor = vec4(Color, alpha);
    if (selectedSegmentID != -1 && vSegmentID == selectedSegmentID) {
        outColor = vec4(1.0, 1.0, 0.0, alpha); // Highlight: Yellow
    }
    FragColor = outColor;
}
