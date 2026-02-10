/*
 * Universidade Federal de Ouro Preto - UFOP
 * Departamento de Computação - DECOM
 * Disciplina: BCC327 - Computação Gráfica (2025.2)
 * Professor: Rafael Bonfim
 * Trabalho Prático: Visualizador de Árvores Arteriais (CCO)
 * Arquivo: Shader.hpp
 * Autor: Mateus Honorato
 * Data: Fevereiro/2026
 * Descrição:
 * Declara a classe Shader para compilação e uso de programas GLSL.
 */

#pragma once

#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader
{
public:
    unsigned int ID;

    Shader(const char *vertexPath, const char *fragmentPath);

    void use() const;
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setMat4(const std::string &name, const glm::mat4 &value) const;

private:
    void checkCompileErrors(unsigned int shader, const std::string &type) const;
};

// fim do arquivo
