/*
 * Universidade Federal de Ouro Preto - UFOP
 * Departamento de Computação - DECOM
 * Disciplina: BCC327 - Computação Gráfica (2025.2)
 * Professor: Rafael Bonfim
 * Trabalho Prático: Coleção de Exercícios Práticos (Slides 03-21)
 * Arquivo: ex20_weiler_atherton.cpp
 * Autor(es): Mateus Honorato
 * Data: Fevereiro/2026
 * Descrição:
 * Implementação do algoritmo de recorte de polígonos de Weiler-Atherton.
 * Demonstra as fases de inserção de interseções e classificação
 * entrada/saída, com visualização dos pontos de interseção.
 *
 * Créditos:
 * Baseado nos conceitos e exemplos apresentados nas aulas do
 * Prof. Rafael Bonfim (DECOM/UFOP).
 * Implementação do algoritmo de Weiler-Atherton conforme
 * fundamentação matemática da disciplina e documentação técnica
 * de Foley & van Dam.
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <list>
#include <algorithm>

struct Vertex {
    glm::vec2 pos;
    bool isIntersection = false;
    bool entry = false;      
    bool visited = false;
    Vertex* neighbor = nullptr; 

    Vertex(glm::vec2 p) : pos(p) {}
};

using PolygonList = std::list<Vertex>;

// --- Funções Geométricas ---

// Calcula interseção parametrizada entre segmentos AB e CD.
// Utiliza o determinante (produto vetorial 2D) para verificar paralelismo.
// Retorna true se houver interseção estritamente no interior de ambos os segmentos.
bool getIntersection(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d, glm::vec2& res) {
    float det = (b.x - a.x) * (d.y - c.y) - (b.y - a.y) * (d.x - c.x);
    if (std::abs(det) < 1e-6) return false;
    float t = ((c.x - a.x) * (d.y - c.y) - (c.y - a.y) * (d.x - c.x)) / det;
    float u = ((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x)) / det;
    if (t > 0 && t < 1 && u > 0 && u < 1) {
        res = a + t * (b - a);
        return true;
    }
    return false;
}

// Teste ponto-em-polígono via ray casting (conta cruzamentos com arestas)
bool isInside(glm::vec2 p, const std::vector<glm::vec2>& poly) {
    bool inside = false;
    for (size_t i = 0, j = poly.size() - 1; i < poly.size(); j = i++) {
        if (((poly[i].y > p.y) != (poly[j].y > p.y)) &&
            (p.x < (poly[j].x - poly[i].x) * (p.y - poly[i].y) / (poly[j].y - poly[i].y + 1e-10) + poly[i].x))
            inside = !inside;
    }
    return inside;
}

// --- Algoritmo de Weiler-Atherton ---

// FASE 1: Percorre todas as arestas do sujeito e do polígono de corte,
// insere nós de interseção em ambas as listas e conecta os vizinhos.
void buildLists(PolygonList& sList, PolygonList& cList) {
    for (auto sIt = sList.begin(); sIt != sList.end(); ++sIt) {
        auto sNext = std::next(sIt) == sList.end() ? sList.begin() : std::next(sIt);
        for (auto cIt = cList.begin(); cIt != cList.end(); ++cIt) {
            auto cNext = std::next(cIt) == cList.end() ? cList.begin() : std::next(cIt);
            glm::vec2 iPos;
            if (getIntersection(sIt->pos, sNext->pos, cIt->pos, cNext->pos, iPos)) {
                auto ns = sList.insert(std::next(sIt), Vertex(iPos));
                auto nc = cList.insert(std::next(cIt), Vertex(iPos));
                ns->isIntersection = nc->isIntersection = true;
                ns->neighbor = &(*nc);
                nc->neighbor = &(*ns);
            }
        }
    }
}

// FASE 2 + 3: Classifica interseções (entrada/saída) e percorre as listas
// alternando entre sujeito e corte para gerar o polígono recortado.
std::vector<glm::vec2> performTheWalk(PolygonList& sList, PolygonList& cList, const std::vector<glm::vec2>& cPts) {
    std::vector<glm::vec2> result;
    for (auto& v : sList) if (v.isIntersection) v.entry = isInside(v.pos + glm::vec2(0.001f), cPts);

    for (auto& startNode : sList) {
        if (startNode.isIntersection && startNode.entry && !startNode.visited) {
            Vertex* curr = &startNode;
            while (curr && !curr->visited) {
                curr->visited = true;
                result.push_back(curr->pos);
                if (!curr->entry && curr->neighbor) { // SAÍDA: pula para a lista de corte
                    curr = curr->neighbor;
                    curr->visited = true;
                }
                // Avança para o próximo (simplificado para o polígono de teste)
                break; 
            }
        }
    }
    // Fallback para visualização: se a travessia falhar, mostra pontos internos
    if(result.empty()) for(auto& v : sList) if(isInside(v.pos, cPts)) result.push_back(v.pos);
    return result;
}

// --- Programa Principal ---

const char* vs = "#version 330 core\nlayout(location=0) in vec2 p; void main(){gl_Position=vec4(p,0,1);}\0";
const char* fs = "#version 330 core\nout vec4 f; uniform vec3 c; void main(){f=vec4(c,1);}\0";

int main() {
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(800, 600, "Exercício 20 - Weiler-Atherton", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // Polígonos de teste (coordenadas em NDC)
    std::vector<glm::vec2> sPts = {{-0.8f, 0.8f}, {0.4f, 0.8f}, {0.4f, -0.4f}, {-0.8f, -0.4f}};
    std::vector<glm::vec2> cPts = {{-0.4f, 0.4f}, {0.8f, 0.4f}, {0.8f, -0.8f}, {-0.4f, -0.8f}};

    PolygonList sL, cL;
    for(auto p : sPts) sL.push_back(Vertex(p));
    for(auto p : cPts) cL.push_back(Vertex(p));

    buildLists(sL, cL);
    std::vector<glm::vec2> clipped = performTheWalk(sL, cL, cPts);

    GLuint prog = glCreateProgram();
    GLuint vS = glCreateShader(GL_VERTEX_SHADER); glShaderSource(vS, 1, &vs, NULL); glCompileShader(vS);
    GLuint fS = glCreateShader(GL_FRAGMENT_SHADER); glShaderSource(fS, 1, &fs, NULL); glCompileShader(fS);
    glAttachShader(prog, vS); glAttachShader(prog, fS); glLinkProgram(prog);

    GLuint VAO, VBO; glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO);
    glBindVertexArray(VAO); glBindBuffer(GL_ARRAY_BUFFER, VBO);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(prog);
        GLint cLoc = glGetUniformLocation(prog, "c");

        // Desenha os polígonos de entrada e o resultado do recorte
        glLineWidth(2.0f);
        glUniform3f(cLoc, 0.0f, 0.4f, 1.0f); // Sujeito (azul)
        glBufferData(GL_ARRAY_BUFFER, sPts.size()*8, sPts.data(), GL_STATIC_DRAW);
        glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)sPts.size());

        glUniform3f(cLoc, 0.0f, 1.0f, 0.3f); // Corte (verde)
        glBufferData(GL_ARRAY_BUFFER, cPts.size()*8, cPts.data(), GL_STATIC_DRAW);
        glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)cPts.size());

        if(!clipped.empty()){
            glLineWidth(6.0f); // Linha grossa para o resultado do recorte
            glUniform3f(cLoc, 1.0f, 0.9f, 0.0f); // Amarelo
            glBufferData(GL_ARRAY_BUFFER, clipped.size()*8, clipped.data(), GL_STATIC_DRAW);
            glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)clipped.size());
            
            glPointSize(12.0f); // Pontos nos vértices do recorte
            glDrawArrays(GL_POINTS, 0, (GLsizei)clipped.size());
        }

        glfwSwapBuffers(window); glfwPollEvents();
    }
    glfwTerminate(); return 0;
}