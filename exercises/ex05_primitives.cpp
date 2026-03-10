/*
 * Universidade Federal de Ouro Preto - UFOP
 * Departamento de Computação - DECOM
 * Disciplina: BCC327 - Computação Gráfica (2025.2)
 * Professor: Rafael Bonfim
 * Exercício Prático: Aula 05 - Primitivas e Atributos
 * Descrição:
 * Renderização de um quadrado colorido utilizando a primitiva
 * GL_TRIANGLE_STRIP com 4 vértices, demonstrando interpolação
 * de cores entre vértices pelo rasterizador.
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

static const char *vertexShaderSrc = R"glsl(#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
out vec3 vColor;
void main() {
    vColor = aColor;
    gl_Position = vec4(aPos, 1.0);
}
)glsl";

static const char *fragmentShaderSrc = R"glsl(#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(vColor, 1.0);
}
)glsl";

// Verifica erros de compilação do shader
static void checkCompile(GLuint shader, const char *name)
{
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char info[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, info);
        std::cerr << name << " erro de compilação:\n"
                  << info << std::endl;
    }
}

// Verifica erros de linkagem do programa
static void checkLink(GLuint prog)
{
    GLint success;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success)
    {
        char info[1024];
        glGetProgramInfoLog(prog, 1024, nullptr, info);
        std::cerr << "Erro de linkagem do programa:\n"
                  << info << std::endl;
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

    GLFWwindow *window = glfwCreateWindow(800, 600, "Exercício 05 - Primitivas", nullptr, nullptr);
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
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Compila e linka o programa de shaders
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSrc, nullptr);
    glCompileShader(vs);
    checkCompile(vs, "Vertex Shader");

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSrc, nullptr);
    glCompileShader(fs);
    checkCompile(fs, "Fragment Shader");

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    checkLink(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    // Formato do vértice: posição (x,y,z) seguido de cor (r,g,b)
    // Quatro vértices na ordem exata para GL_TRIANGLE_STRIP formar um quadrado:
    // V0: (-0.5, 0.5)  - Vermelho
    // V1: (-0.5,-0.5)  - Verde
    // V2: ( 0.5, 0.5)  - Azul
    // V3: ( 0.5,-0.5)  - Amarelo
    float vertices[] = {
        // posições          // cores
        -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f,  // V0
        -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // V1
        0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f,   // V2
        0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f   // V3
    };

    // Configura VAO/VBO para armazenar a geometria na GPU
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Atributo 0: posição (3 floats)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // Atributo 1: cor (3 floats, deslocamento de 12 bytes)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        glClear(GL_COLOR_BUFFER_BIT);

        // Rasteriza o quadrado como TRIANGLE_STRIP: o rasterizador interpola
        // as cores entre os 4 vértices automaticamente
        glUseProgram(program);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(program);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
