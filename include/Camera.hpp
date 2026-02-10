/*
 * Universidade Federal de Ouro Preto - UFOP
 * Departamento de Computação - DECOM
 * Disciplina: BCC327 - Computação Gráfica (2025.2)
 * Professor: Rafael Bonfim
 * Trabalho Prático: Visualizador de Árvores Arteriais (CCO)
 * Arquivo: Camera.hpp
 * Autor: Mateus Honorato
 * Data: Fevereiro/2026
 * Descrição:
 * Declara a classe Camera responsável pela matriz de visualização
 * e pelo processamento de entrada do mouse (pan/rotate/zoom).
 *
 * Créditos:
 * A lógica de processamento de mouse (Euler Angles: Yaw/Pitch) e movimentação
 * foi adaptada da classe 'Camera' do tutorial LearnOpenGL.com (Joey de Vries).
 */

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
private:
    glm::vec3 panOffset = glm::vec3(0.0f);
    float distance = 5.0f;
    float yaw = -90.0f;
    float pitch = 0.0f;
    float sensitivity = 0.5f;
    bool isDragging = false;
    bool isPanning = false;
    float lastX = 400.0f, lastY = 300.0f;

public:
    Camera();
    glm::mat4 getViewMatrix() const;
    glm::vec3 getPosition() const;
    void processMouseScroll(float yoffset);
    void processMouseButton(int button, int action, double xpos, double ypos);
    void processMouseMovement(double xpos, double ypos);
    void setDistance(float d);
    void processMousePan(float xoffset, float yoffset);
    void reset();
};
