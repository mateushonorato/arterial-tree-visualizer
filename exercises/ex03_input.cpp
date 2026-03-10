/*
 * Universidade Federal de Ouro Preto - UFOP
 * Departamento de Computação - DECOM
 * Disciplina: BCC327 - Computação Gráfica (2025.2)
 * Professor: Rafael Bonfim
 * Exercício Prático: Aula 03 - Fundamentos de Programação Gráfca
 * Descrição:
 * Demonstração do tratamento de entrada via teclado em OpenGL
 * moderno com GLFW. As teclas R e B alteram a cor de fundo da
 * janela, ilustrando o callback de teclado e o loop de eventos.
 *
 * Créditos:
 * Baseado nos conceitos e exemplos apresentados nas aulas do
 * Prof. Rafael Bonfim (DECOM/UFOP).
 * Estrutura base de inicialização do GLFW/GLAD e compilação de
 * shaders (Core Profile 3.3) adaptada do tutorial LearnOpenGL.com.
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// Cor de limpeza do framebuffer (azul por padrão)
static float clearColor[3] = {0.0f, 0.0f, 1.0f};

// Callback de teclado: GLFW invoca esta função sempre que uma tecla é pressionada/solta
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_ESCAPE)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        else if (key == GLFW_KEY_R)
        {
            clearColor[0] = 1.0f;
            clearColor[1] = 0.0f;
            clearColor[2] = 0.0f; // Vermelho
        }
        else if (key == GLFW_KEY_B)
        {
            clearColor[0] = 0.0f;
            clearColor[1] = 0.0f;
            clearColor[2] = 1.0f; // Azul
        }
    }
}

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Falha ao inicializar GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "Exercício 03 - Entrada", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Falha ao criar janela GLFW\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Falha ao inicializar GLAD\n";
        return -1;
    }

    // Registra o callback de teclado antes do loop principal
    glfwSetKeyCallback(window, key_callback);

    // Loop principal: limpa o framebuffer com a cor atual e processa eventos
    while (!glfwWindowShouldClose(window))
    {
        glViewport(0, 0, 800, 600);
        glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
