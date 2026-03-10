/*
 * Universidade Federal de Ouro Preto - UFOP
 * Departamento de Computação - DECOM
 * Disciplina: BCC327 - Computação Gráfica (2025.2)
 * Professor: Rafael Bonfim
 * Exercício Prático: Aula 19 - Recorte de Polígonos - Algoritmo de Sutherland–Hodgman
 * Descrição:
 * Implementação do algoritmo de recorte de polígonos de Sutherland-Hodgman.
 * Recorta um triângulo contra uma janela retangular iterando sobre as
 * 4 bordas (esquerda, direita, inferior, superior), aplicando as 4 regras
 * de transição em cada aresta. Usa struct Point e std::vector<Point>.
 *
 * Créditos:
 * Baseado nos conceitos e exemplos apresentados nas aulas do
 * Prof. Rafael Bonfim (DECOM/UFOP).
 * Implementação do algoritmo de Sutherland-Hodgman conforme
 * fundamentação matemática da disciplina e documentação técnica
 * de Foley & van Dam.
 * Estrutura base de inicialização (GLFW/GLAD) e manipulação
 * vetorial/matricial adaptada da biblioteca GLM e do tutorial LearnOpenGL.com.
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>

// --- Estrutura de Ponto ---

struct Point
{
    float x, y;
};

// --- Shaders GLSL 330 Core ---

static const char *vertexShaderSrc = R"glsl(#version 330 core
layout(location = 0) in vec2 aPos;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)glsl";

static const char *fragmentShaderSrc = R"glsl(#version 330 core
out vec4 FragColor;
uniform vec3 uColor;
void main() {
    FragColor = vec4(uColor, 1.0);
}
)glsl";

// --- Funções Auxiliares do Sutherland-Hodgman ---

// Tipos de borda da janela de recorte
enum Edge
{
    LEFT,
    RIGHT,
    BOTTOM,
    TOP
};

// Verifica se um ponto está do lado "dentro" de uma borda da janela.
// Para cada borda, a condição de inclusão é:
//   Esquerda:  x >= xMin
//   Direita:   x <= xMax
//   Inferior:  y >= yMin
//   Superior:  y <= yMax
static bool isInside(const Point &p, Edge edge,
                     float xMin, float xMax, float yMin, float yMax)
{
    switch (edge)
    {
    case LEFT:
        return p.x >= xMin;
    case RIGHT:
        return p.x <= xMax;
    case BOTTOM:
        return p.y >= yMin;
    case TOP:
        return p.y <= yMax;
    }
    return false;
}

// Calcula o ponto de interseção entre o segmento (A → B) e uma borda
// da janela de recorte, usando a equação paramétrica da reta:
//
//   P(t) = A + t * (B - A),  onde t ∈ [0, 1]
//
// Para a borda vertical x = k (esquerda ou direita):
//   t = (k - A.x) / (B.x - A.x)
//   I.x = k
//   I.y = A.y + t * (B.y - A.y)
//
// Para a borda horizontal y = k (inferior ou superior):
//   t = (k - A.y) / (B.y - A.y)
//   I.x = A.x + t * (B.x - A.x)
//   I.y = k
static Point computeIntersection(const Point &a, const Point &b, Edge edge,
                                 float xMin, float xMax, float yMin, float yMax)
{
    Point i;
    float dx = b.x - a.x;
    float dy = b.y - a.y;

    switch (edge)
    {
    case LEFT:
    {
        // Interseção com a borda esquerda: x = xMin
        float t = (xMin - a.x) / dx;
        i.x = xMin;
        i.y = a.y + t * dy;
        break;
    }
    case RIGHT:
    {
        // Interseção com a borda direita: x = xMax
        float t = (xMax - a.x) / dx;
        i.x = xMax;
        i.y = a.y + t * dy;
        break;
    }
    case BOTTOM:
    {
        // Interseção com a borda inferior: y = yMin
        float t = (yMin - a.y) / dy;
        i.x = a.x + t * dx;
        i.y = yMin;
        break;
    }
    case TOP:
    {
        // Interseção com a borda superior: y = yMax
        float t = (yMax - a.y) / dy;
        i.x = a.x + t * dx;
        i.y = yMax;
        break;
    }
    }
    return i;
}

// Recorta uma lista de vértices contra UMA borda da janela, produzindo
// uma nova lista de vértices. Para cada aresta (S → P) do polígono de
// entrada, aplica as 4 regras de transição do Sutherland-Hodgman:
//
//   Regra 1: S dentro,  P dentro  → emite P
//   Regra 2: S dentro,  P fora   → emite interseção I
//   Regra 3: S fora,    P fora   → não emite nada
//   Regra 4: S fora,    P dentro  → emite interseção I e depois P
static std::vector<Point> clipAgainstEdge(const std::vector<Point> &polygon, Edge edge,
                                          float xMin, float xMax, float yMin, float yMax)
{
    std::vector<Point> output;
    if (polygon.empty())
        return output;

    size_t n = polygon.size();
    for (size_t i = 0; i < n; ++i)
    {
        // S é o vértice anterior, P é o vértice atual
        const Point &S = polygon[(i + n - 1) % n];
        const Point &P = polygon[i];

        bool sInside = isInside(S, edge, xMin, xMax, yMin, yMax);
        bool pInside = isInside(P, edge, xMin, xMax, yMin, yMax);

        if (sInside && pInside)
        {
            // Regra 1: ambos dentro → emite P
            output.push_back(P);
        }
        else if (sInside && !pInside)
        {
            // Regra 2: S dentro, P fora → emite apenas a interseção
            output.push_back(computeIntersection(S, P, edge, xMin, xMax, yMin, yMax));
        }
        else if (!sInside && !pInside)
        {
            // Regra 3: ambos fora → não emite nada
        }
        else
        {
            // Regra 4: S fora, P dentro → emite interseção e depois P
            output.push_back(computeIntersection(S, P, edge, xMin, xMax, yMin, yMax));
            output.push_back(P);
        }
    }
    return output;
}

// Algoritmo completo de Sutherland-Hodgman: recorta o polígono de entrada
// iterativamente contra cada uma das 4 bordas da janela retangular.
// A saída de cada etapa torna-se a entrada da próxima.
std::vector<Point> sutherlandHodgman(const std::vector<Point> &polygon,
                                     float xMin, float xMax, float yMin, float yMax)
{
    std::vector<Point> result = polygon;

    // Itera sobre as 4 bordas: esquerda, direita, inferior, superior
    result = clipAgainstEdge(result, LEFT, xMin, xMax, yMin, yMax);
    result = clipAgainstEdge(result, RIGHT, xMin, xMax, yMin, yMax);
    result = clipAgainstEdge(result, BOTTOM, xMin, xMax, yMin, yMax);
    result = clipAgainstEdge(result, TOP, xMin, xMax, yMin, yMax);

    return result;
}

// --- Programa Principal ---

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(800, 600,
                                          "Exercício 19 - Sutherland-Hodgman", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // --- Geometria de Teste ---

    // Triângulo original (vermelho)
    std::vector<Point> triangle = {
        {-0.8f, -0.8f},
        {0.8f, -0.8f},
        {0.0f, 0.9f}};

    // Janela de recorte: x ∈ [-0.5, 0.5], y ∈ [-0.5, 0.5]
    float xMin = -0.5f, xMax = 0.5f;
    float yMin = -0.5f, yMax = 0.5f;

    // Executa o algoritmo de Sutherland-Hodgman
    std::vector<Point> clipped = sutherlandHodgman(triangle, xMin, xMax, yMin, yMax);

    // Vértices da janela de recorte (para desenhar o contorno)
    std::vector<Point> clipRect = {
        {xMin, yMin}, {xMax, yMin}, {xMax, yMax}, {xMin, yMax}};

    // Diagnóstico no terminal
    std::cout << "Polígono recortado (" << clipped.size() << " vértices):" << std::endl;
    for (const auto &p : clipped)
        std::cout << "  (" << p.x << ", " << p.y << ")" << std::endl;

    // --- Compilação dos Shaders ---
    GLuint prog = glCreateProgram();
    GLuint vS = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vS, 1, &vertexShaderSrc, NULL);
    glCompileShader(vS);
    GLuint fS = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fS, 1, &fragmentShaderSrc, NULL);
    glCompileShader(fS);
    glAttachShader(prog, vS);
    glAttachShader(prog, fS);
    glLinkProgram(prog);
    glDeleteShader(vS);
    glDeleteShader(fS);

    // --- Configuração dos Buffers Geométricos (VAO/VBO) ---
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Point), (void *)0);
    glEnableVertexAttribArray(0);

    // --- Loop de Renderização ---
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(prog);
        GLint colorLoc = glGetUniformLocation(prog, "uColor");

        // 1. Desenha o triângulo original em VERMELHO (wireframe, GL_LINE_LOOP)
        glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f);
        glBufferData(GL_ARRAY_BUFFER,
                     triangle.size() * sizeof(Point),
                     triangle.data(), GL_DYNAMIC_DRAW);
        glLineWidth(2.0f);
        glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)triangle.size());

        // 2. Desenha a janela de recorte em BRANCO (wireframe)
        glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
        glBufferData(GL_ARRAY_BUFFER,
                     clipRect.size() * sizeof(Point),
                     clipRect.data(), GL_DYNAMIC_DRAW);
        glLineWidth(1.0f);
        glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)clipRect.size());

        // 3. Desenha o resultado do recorte em VERDE (preenchido, GL_TRIANGLE_FAN)
        if (!clipped.empty())
        {
            glUniform3f(colorLoc, 0.0f, 1.0f, 0.0f);
            glBufferData(GL_ARRAY_BUFFER,
                         clipped.size() * sizeof(Point),
                         clipped.data(), GL_DYNAMIC_DRAW);
            glDrawArrays(GL_TRIANGLE_FAN, 0, (GLsizei)clipped.size());
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- Limpeza ---
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(prog);
    glfwTerminate();
    return 0;
}
