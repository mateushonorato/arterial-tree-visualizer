#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <list>

// Estrutura de Vértice para as Listas do Weiler-Atherton
struct Vertex {
    glm::vec2 pos;
    bool isIntersection = false;
    bool entry = false;      // true = Entrada, false = Saída
    bool visited = false;
    Vertex* neighbor = nullptr; // Ponteiro para o "irmão" na outra lista

    Vertex(glm::vec2 p) : pos(p) {}
};

typedef std::list<Vertex> PolygonList;

// --- Funções Matemáticas de Apoio ---

bool getIntersection(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d, glm::vec2& res) {
    float det = (b.x - a.x) * (d.y - c.y) - (b.y - a.y) * (d.x - c.x);
    if (std::abs(det) < 1e-6) return false; // Paralelas

    float t = ((c.x - a.x) * (d.y - c.y) - (c.y - a.y) * (d.x - c.x)) / det;
    float u = ((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x)) / det;

    if (t > 0 && t < 1 && u > 0 && u < 1) {
        res = a + t * (b - a);
        return true;
    }
    return false;
}

bool isInside(glm::vec2 p, const std::vector<glm::vec2>& poly) {
    bool inside = false;
    for (size_t i = 0, j = poly.size() - 1; i < poly.size(); j = i++) {
        if (((poly[i].y > p.y) != (poly[j].y > p.y)) &&
            (p.x < (poly[j].x - poly[i].x) * (p.y - poly[i].y) / (poly[j].y - poly[i].y) + poly[i].x))
            inside = !inside;
    }
    return inside;
}

// --- Lógica do Algoritmo ---

void buildLists(const std::vector<glm::vec2>& sPoints, const std::vector<glm::vec2>& cPoints, 
                PolygonList& sList, PolygonList& cList) {
    for (auto p : sPoints) sList.emplace_back(p);
    for (auto p : cPoints) cList.emplace_back(p);

    // Encontrar e inserir interseções
    for (auto sIt = sList.begin(); sIt != sList.end(); ++sIt) {
        auto sNext = std::next(sIt) == sList.end() ? sList.begin() : std::next(sIt);
        for (auto cIt = cList.begin(); cIt != cList.end(); ++cIt) {
            auto cNext = std::next(cIt) == cList.end() ? cList.begin() : std::next(cIt);

            glm::vec2 interPos;
            if (getIntersection(sIt->pos, sNext->pos, cIt->pos, cNext->pos, interPos)) {
                Vertex sInter(interPos);
                sInter.isIntersection = true;
                Vertex cInter(interPos);
                cInter.isIntersection = true;

                auto newS = sList.insert(sNext, sInter);
                auto newC = cList.insert(cNext, cInter);
                
                newS->neighbor = &(*newC);
                newC->neighbor = &(*newS);
            }
        }
    }
}

// Classifica se a interseção é de entrada ou saída no Polígono de Corte
void classify(PolygonList& sList, const std::vector<glm::vec2>& cPoints) {
    for (auto& v : sList) {
        if (v.isIntersection) {
            // Se o ponto anterior estava fora, esta é uma ENTRADA
            // (Simplificação: verificamos o ponto médio do segmento anterior)
            v.entry = isInside(v.pos, cPoints); 
        }
    }
}

// --- Renderização ---

const char* vs = "#version 330 core\nlayout(location=0) in vec2 p; void main(){gl_Position=vec4(p,0,1);}\0";
const char* fs = "#version 330 core\nout vec4 f; uniform vec3 c; void main(){f=vec4(c,1);}\0";

int main() {
    glfwInit();
    GLFWwindow* win = glfwCreateWindow(800, 600, "WA Clipping - BCC327", NULL, NULL);
    glfwMakeContextCurrent(win);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // Definição original
    std::vector<glm::vec2> sPoints = { {-0.6, 0.4}, {0.2, 0.4}, {0.2, -0.4}, {-0.6, -0.4} };
    std::vector<glm::vec2> cPoints = { {-0.3, 0.2}, {0.5, 0.2}, {0.5, -0.6}, {-0.3, -0.6} };

    PolygonList sList, cList;
    buildLists(sPoints, cPoints, sList, cList);
    classify(sList, cPoints);

    // Gerar lista para desenho
    std::vector<glm::vec2> result;
    for (auto& v : sList) if (isInside(v.pos, cPoints)) result.push_back(v.pos);

    GLuint shader = glCreateProgram();
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vs, NULL); glCompileShader(vShader);
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fs, NULL); glCompileShader(fShader);
    glAttachShader(shader, vShader); glAttachShader(shader, fShader); glLinkProgram(shader);

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    while (!glfwWindowShouldClose(win)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shader);
        GLint cLoc = glGetUniformLocation(shader, "c");

        // Desenhar Sujeito (Azul)
        glUniform3f(cLoc, 0, 0.5, 1);
        glBufferData(GL_ARRAY_BUFFER, sPoints.size()*8, sPoints.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8, 0); glEnableVertexAttribArray(0);
        glDrawArrays(GL_LINE_LOOP, 0, sPoints.size());

        // Desenhar Corte (Verde)
        glUniform3f(cLoc, 0, 1, 0);
        glBufferData(GL_ARRAY_BUFFER, cPoints.size()*8, cPoints.data(), GL_STATIC_DRAW);
        glDrawArrays(GL_LINE_LOOP, 0, cPoints.size());

        // Desenhar RESULTADO (Amarelo - Grosso)
        if(!result.empty()) {
            glLineWidth(4.0f);
            glUniform3f(cLoc, 1, 1, 0);
            glBufferData(GL_ARRAY_BUFFER, result.size()*8, result.data(), GL_STATIC_DRAW);
            glDrawArrays(GL_POINTS, 0, result.size()); // Mostra os pontos internos
        }

        glfwSwapBuffers(win); glfwPollEvents();
    }
    glfwTerminate(); return 0;
}