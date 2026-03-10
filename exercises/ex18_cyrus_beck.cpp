/*
 * Universidade Federal de Ouro Preto - UFOP
 * Departamento de Computação - DECOM
 * Disciplina: BCC327 - Computação Gráfica (2025.2)
 * Professor: Rafael Bonfim
 * Exercício Prático: Aula 18 - Recorte de Segmentos de Retas - Algoritmo Cyrus-Beck
 * Descrição:
 * Recorte interativo de segmentos de reta contra um polígono convexo
 * (hexágono regular) usando o algoritmo de Cyrus-Beck.
 * Clique esquerdo define P0, clique direito define P1, tecla C limpa.
 *
 * Créditos:
 * Baseado nos conceitos e exemplos apresentados nas aulas do
 * Prof. Rafael Bonfim (DECOM/UFOP).
 * Implementação do algoritmo de Cyrus-Beck conforme fundamentação
 * matemática da disciplina e documentação técnica de Foley & van Dam.
 * Estrutura base de inicialização (GLFW/GLAD) e manipulação 
 * vetorial/matricial adaptada da biblioteca GLM e do tutorial LearnOpenGL.com.
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>

static int SCR_W = 800;
static int SCR_H = 600;

static glm::vec2 P0, P1;
static bool hasP0 = false, hasP1 = false;

// Shader simples com cor uniforme
static const char *vs_src = R"glsl(#version 330 core
layout(location = 0) in vec2 aPos;
uniform mat4 proj;
void main(){ gl_Position = proj * vec4(aPos, 0.0, 1.0); }
)glsl";

static const char *fs_src = R"glsl(#version 330 core
uniform vec3 uColor;
out vec4 FragColor;
void main(){ FragColor = vec4(uColor,1.0); }
)glsl";

// Converte a coordenada da tela para o sistema Normalizado de Coordenadas do Dispositivo (NDC)
static void cursor_to_ndc(GLFWwindow *w, double x, double y, glm::vec2 &out)
{
    out.x = (float)((2.0 * x) / SCR_W - 1.0);
    out.y = (float)(1.0 - (2.0 * y) / SCR_H);
}

// Controle de Interação com o Mouse
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        cursor_to_ndc(window, x, y, P0);
        hasP0 = true;
        if (!hasP1)
            P1 = P0; // Evita desenhar "lixo" na memória se P1 não existir
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        cursor_to_ndc(window, x, y, P1);
        hasP1 = true;
    }
}

// Controle do Teclado
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_C)
        {
            hasP0 = false;
            hasP1 = false;
        }
        else if (key == GLFW_KEY_ESCAPE)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
}

// Algoritmo Paramétrico de Recorte (Cyrus-Beck)
std::pair<bool, std::pair<glm::vec2, glm::vec2>> cyrus_beck_clip(glm::vec2 p0, glm::vec2 p1, const std::vector<glm::vec2> &poly)
{
    float tE = 0.0f; // Parâmetro Máximo de Entrada
    float tL = 1.0f; // Parâmetro Mínimo de Saída
    glm::vec2 d = p1 - p0;

    // Se a reta for um único ponto
    if (d.x == 0.0f && d.y == 0.0f)
        return {false, {}};

    // Verifica todas as arestas do polígono
    for (size_t i = 0; i < poly.size(); ++i)
    {
        glm::vec2 p_edge = poly[i];
        glm::vec2 p_next = poly[(i + 1) % poly.size()];

        // Calcula vetor da aresta e Normal (apontando para DENTRO - giro anti-horário de 90 graus)
        glm::vec2 edge = p_next - p_edge;
        glm::vec2 n(-edge.y, edge.x);

        // Produto escalar para determinar o peso direcional
        glm::vec2 w = p0 - p_edge;
        float num = glm::dot(w, n);
        float den = glm::dot(d, n);

        if (den == 0.0f)
        {
            if (num < 0.0f)
                return {false, {}}; // Linha é paralela à aresta e está do lado de fora
        }
        else if (den > 0)
        {
            // den > 0: A reta está ENTRANDO no polígono
            float t = -num / den;
            if (t > tL)
                return {false, {}};
            if (t > tE)
                tE = t;
        }
        else
        {
            // den < 0: A reta está SAINDO do polígono
            float t = -num / den;
            if (t < tE)
                return {false, {}};
            if (t < tL)
                tL = t;
        }
    }

    // Se t de Entrada ultrapassar o t de Saída, a reta está inteiramente fora
    if (tE > tL)
        return {false, {}};

    // Calcula os novos pontos usando a equação paramétrica da reta: P(t) = P0 + t * D
    glm::vec2 p0_clip = p0 + tE * d;
    glm::vec2 p1_clip = p0 + tL * d;

    return {true, {p0_clip, p1_clip}};
}

int main()
{
    if (!glfwInit())
        return -1;
    GLFWwindow *window = glfwCreateWindow(SCR_W, SCR_H, "Exercício 18 - Cyrus-Beck", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwTerminate();
        return -1;
    }

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);

    // Definição do Polígono de Corte (Hexágono Regular Convexo)
    std::vector<glm::vec2> hex;
    for (int i = 0; i < 6; ++i)
    {
        float ang = glm::radians(i * 60.0f);
        hex.push_back(glm::vec2(0.5f * cos(ang), 0.5f * sin(ang)));
    }

    // Compilação dos Shaders
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_src, nullptr);
    glCompileShader(vs);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_src, nullptr);
    glCompileShader(fs);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);

    // Configuração dos Buffers Geométricos (VAO/VBO)
    GLuint polyVAO, polyVBO;
    glGenVertexArrays(1, &polyVAO);
    glGenBuffers(1, &polyVBO);
    glBindVertexArray(polyVAO);
    glBindBuffer(GL_ARRAY_BUFFER, polyVBO);
    glBufferData(GL_ARRAY_BUFFER, hex.size() * sizeof(glm::vec2), hex.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)0);
    glEnableVertexAttribArray(0);

    GLuint lineVAO, lineVBO;
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec2), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)0);
    glEnableVertexAttribArray(0);

    glUseProgram(prog);
    GLint colorLoc = glGetUniformLocation(prog, "uColor");
    GLint projLoc = glGetUniformLocation(prog, "proj");

    // Projeção Ortográfica (Mapeia o NDC diretamente)
    glm::mat4 proj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

    while (!glfwWindowShouldClose(window))
    {
        glfwGetFramebufferSize(window, &SCR_W, &SCR_H);
        glViewport(0, 0, SCR_W, SCR_H);

        glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Desenha o contorno do polígono de corte em branco
        glBindVertexArray(polyVAO);
        glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
        glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)hex.size());

        // Desenha o segmento de reta (se os pontos iniciais estiverem marcados)
        if (hasP0 && hasP1)
        {
            glm::vec2 seg[2] = {P0, P1};
            glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(seg), seg);
            glBindVertexArray(lineVAO);

            // Renderiza linha original (Fina e Cinza)
            glLineWidth(1.0f);
            glUniform3f(colorLoc, 0.5f, 0.5f, 0.5f);
            glDrawArrays(GL_LINES, 0, 2);

            // Executa o cálculo analítico do Cyrus-Beck
            auto res = cyrus_beck_clip(P0, P1, hex);
            if (res.first)
            {
                glm::vec2 clipped[2] = {res.second.first, res.second.second};
                glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(clipped), clipped);
                glBindVertexArray(lineVAO);

                // Renderiza a porção interna recortada (Grossa e Vermelha)
                glLineWidth(4.0f);
                glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f);
                glDrawArrays(GL_LINES, 0, 2);
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}