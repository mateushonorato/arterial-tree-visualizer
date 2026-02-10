/*
 * Universidade Federal de Ouro Preto - UFOP
 * Departamento de Computação - DECOM
 * Disciplina: BCC327 - Computação Gráfica (2025.2)
 * Professor: Rafael Bonfim
 * Trabalho Prático: Visualizador de Árvores Arteriais (CCO)
 * Arquivo: Camera.cpp
 * Autor: Mateus Honorato
 * Data: Fevereiro/2026
 * Descrição:
 * Implementa a câmera orbital com suporte a rotação, pan e zoom.
 *
 * Créditos:
 * A lógica de processamento de mouse (Euler Angles: Yaw/Pitch) e movimentação
 * foi adaptada da classe 'Camera' do tutorial LearnOpenGL.com (Joey de Vries).
 */

#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.hpp"

Camera::Camera()
{
    yaw = 90.0f;
    pitch = 0.0f;
    distance = 5.0f;
}

glm::vec3 Camera::getPosition() const
{
    // Converte ângulos esféricos (yaw, pitch) para coordenadas cartesianas.
    // `yaw` é a rotação em torno do eixo Y e `pitch` inclina acima/abaixo;
    // usando seno e cosseno obtemos a posição no espaço 3D a partir da
    // distância radial (radius = distance).
    float yawRad = glm::radians(yaw);
    float pitchRad = glm::radians(pitch);
    float x = distance * cosf(pitchRad) * cosf(yawRad);
    float y = distance * sinf(pitchRad);
    float z = distance * cosf(pitchRad) * sinf(yawRad);
    return glm::vec3(x, y, z);
}

glm::mat4 Camera::getViewMatrix() const
{
    // 1. Calcula a matriz de View padrão (Orbitando o centro 0,0,0)
    glm::vec3 position = getPosition();
    glm::mat4 view = glm::lookAt(position, glm::vec3(0.0f), glm::vec3(0, 1, 0));

    // 2. Aplica o Pan no ESPAÇO DA TELA (Multiplicação pela Esquerda)
    // Isso garante que a translação aconteça nos eixos da câmera (X=Direita, Y=Cima)
    // independente da rotação.
    glm::mat4 panTransform = glm::translate(glm::mat4(1.0f), panOffset);
    return panTransform * view;
}

void Camera::processMousePan(float xoffset, float yoffset)
{
    // Ajuste fino da velocidade (sensibilidade)
    float velocity = sensitivity * distance * 0.001f;

    // Manipulação direta: Arrastar para Direita (x+) move objeto para Direita (x+)
    panOffset.x += xoffset * velocity;

    // Arrastar para Cima (y+) move objeto para Cima (y+)
    // O sinal depende de como yoffset é calculado. Assumindo (lastY - ypos) = Up positivo.
    panOffset.y += yoffset * velocity;
}

void Camera::reset()
{
    panOffset = glm::vec3(0.0f);
    distance = 5.0f;
    yaw = 90.0f;
    pitch = 0.0f;
}

void Camera::processMouseScroll(float yoffset)
{
    if (yoffset > 0)
        distance *= 0.9f;
    else
        distance *= 1.1f;
    distance = std::clamp(distance, 0.01f, 50.0f);
}

void Camera::processMouseButton(int button, int action, double xpos, double ypos)
{
    if (button == 0)
    { // Esquerdo (Rotação)
        if (action == 1)
        {
            isDragging = true;
            lastX = static_cast<float>(xpos);
            lastY = static_cast<float>(ypos);
        }
        else
        {
            isDragging = false;
        }
    }
    if (button == 1)
    { // Direito (Pan)
        if (action == 1)
        {
            isPanning = true;
            lastX = static_cast<float>(xpos);
            lastY = static_cast<float>(ypos);
        }
        else
        {
            isPanning = false;
        }
    }
}

void Camera::processMouseMovement(double xpos, double ypos)
{
    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos); // Y cresce para cima

    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    if (isPanning)
    {
        processMousePan(xoffset, yoffset);
        return;
    }

    if (isDragging)
    {
        yaw += xoffset * sensitivity;
        pitch += yoffset * sensitivity;
        pitch = std::clamp(pitch, -89.0f, 89.0f);
    }
}

void Camera::setDistance(float d)
{
    distance = std::clamp(d, 0.1f, 50.0f);
}