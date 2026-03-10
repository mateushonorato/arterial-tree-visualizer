/*
 * Universidade Federal de Ouro Preto - UFOP
 * Departamento de Computação - DECOM
 * Disciplina: BCC327 - Computação Gráfica (2025.2)
 * Professor: Rafael Bonfim
 * Exercício Prático: Aula 04 - Teoria das Cores e Modelos de Cor
 * Descrição:
 * Demonstração da soma aditiva de cores (Vermelho + Verde = Amarelo)
 * utilizando um uniform vec3 no fragment shader para definir a cor
 * do triângulo renderizado.
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
#include <string>

static const char *vertexShaderSrc = R"glsl(#version 330 core
layout(location = 0) in vec3 aPos;
void main() {
    gl_Position = vec4(aPos, 1.0);
}
)glsl";

static const char *fragmentShaderSrc = R"glsl(#version 330 core
out vec4 FragColor;
uniform vec3 triangleColor;
void main() {
    FragColor = vec4(triangleColor, 1.0);
}
)glsl";

// Verifica erros de compilação do shader
static void checkCompile(GLuint shader, const std::string &name)
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

    GLFWwindow *window = glfwCreateWindow(800, 600, "Exercício 04 - Soma de Cores", nullptr, nullptr);
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

    // Compila e linka o programa de shaders (pipeline programável)
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

    // Vértices do triângulo em coordenadas NDC (Normalized Device Coordinates)
    float vertices[] = {
        0.0f, 0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f};

    // Configura VAO/VBO para armazenar a geometria na GPU
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Define a cor de limpeza do framebuffer
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    // Soma aditiva: Vermelho (1,0,0) + Verde (0,1,0) = Amarelo (1,1,0)
    glUseProgram(program);
    GLint colorLoc = glGetUniformLocation(program, "triangleColor");
    if (colorLoc != -1)
    {
        glUniform3f(colorLoc, 1.0f, 1.0f, 0.0f);
    }

    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        glClear(GL_COLOR_BUFFER_BIT);

        // Envia os vértices para a rasterização com base no VAO ativo
        glUseProgram(program);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

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
