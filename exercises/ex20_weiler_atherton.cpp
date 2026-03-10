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
 * Demonstra as fases de inserção de interseções, classificação
 * entrada/saída e travessia (The Walk), com visualização do resultado.
 * Inclui teste com polígono sujeito contendo um buraco (ponte/bridge).
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
#include <iostream>
#include <list>
#include <algorithm>
#include <cmath>

// --- Estruturas ---

// Vértice aumentado para as listas encadeadas do Weiler-Atherton.
// Armazena posição, marcadores de interseção/entrada e ponteiro
// para o vértice correspondente na outra lista (portal de travessia).
struct Vertex {
    glm::vec2 pos;
    bool isIntersection = false;
    bool entry = false;       // true = Entrada no clipper, false = Saída
    bool visited = false;
    Vertex* neighbor = nullptr; // Ponteiro para o vértice correspondente na outra lista

    Vertex(glm::vec2 p) : pos(p) {}
};

using PolygonList = std::list<Vertex>;

// Avança iterador circularmente na std::list (wrap-around ao fim)
inline PolygonList::iterator circularNext(PolygonList& L, PolygonList::iterator it) {
    ++it;
    return (it == L.end()) ? L.begin() : it;
}

// --- Funções Geométricas ---

// Calcula interseção parametrizada entre segmentos AB e CD.
// Resolve o sistema linear via determinante (produto vetorial 2D):
//   det = (B-A) × (D-C)
// Os parâmetros t (ao longo AB) e u (ao longo CD) definem I = A + t*(B-A).
// Retorna true se a interseção for estritamente interior a ambos os segmentos.
bool getIntersection(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d,
                     glm::vec2& res, float& t, float& u) {
    float det = (b.x - a.x) * (d.y - c.y) - (b.y - a.y) * (d.x - c.x);
    if (std::abs(det) < 1e-6f) return false; // Segmentos (quase) paralelos
    t = ((c.x - a.x) * (d.y - c.y) - (c.y - a.y) * (d.x - c.x)) / det;
    u = ((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x)) / det;
    if (t > 1e-6f && t < 1.0f - 1e-6f && u > 1e-6f && u < 1.0f - 1e-6f) {
        res = a + t * (b - a);
        return true;
    }
    return false;
}

// Teste ponto-em-polígono via ray casting (contagem de cruzamentos).
// Lança um raio horizontal para a direita a partir de p e conta quantas
// arestas do polígono ele cruza. Número ímpar → dentro, par → fora.
bool isInside(glm::vec2 p, const std::vector<glm::vec2>& poly) {
    bool inside = false;
    for (size_t i = 0, j = poly.size() - 1; i < poly.size(); j = i++) {
        if (((poly[i].y > p.y) != (poly[j].y > p.y)) &&
            (p.x < (poly[j].x - poly[i].x) * (p.y - poly[i].y) /
                    (poly[j].y - poly[i].y) + poly[i].x))
            inside = !inside;
    }
    return inside;
}

// --- FASE 1: Construção das Listas Encadeadas Aumentadas ---
// Encontra TODAS as interseções entre arestas do sujeito e do clipper,
// ordena-as pelo parâmetro t (ou u) ao longo de cada aresta e insere
// os vértices de interseção nas posições corretas de cada std::list.
// Conecta os pares de interseção via ponteiros de memória (neighbor).

struct IsectRecord {
    glm::vec2 pos;
    float tS, tC;       // Parâmetros ao longo das arestas do sujeito e clipper
    int sEdge, cEdge;   // Índices das arestas originais
};

void buildAugmentedLists(
    const std::vector<glm::vec2>& subject,
    const std::vector<glm::vec2>& clipper,
    PolygonList& sList,
    PolygonList& cList)
{
    int nS = (int)subject.size();
    int nC = (int)clipper.size();

    // Popula as listas encadeadas com os vértices originais
    for (int i = 0; i < nS; i++) sList.push_back(Vertex(subject[i]));
    for (int i = 0; i < nC; i++) cList.push_back(Vertex(clipper[i]));

    // Encontra todas as interseções aresta-a-aresta
    std::vector<IsectRecord> isects;
    for (int si = 0; si < nS; si++) {
        glm::vec2 sA = subject[si], sB = subject[(si + 1) % nS];
        for (int ci = 0; ci < nC; ci++) {
            glm::vec2 cA = clipper[ci], cB = clipper[(ci + 1) % nC];
            glm::vec2 iPos; float t, u;
            if (getIntersection(sA, sB, cA, cB, iPos, t, u))
                isects.push_back({iPos, t, u, si, ci});
        }
    }

    // Agrupa interseções por aresta e ordena pelo parâmetro ao longo dela
    std::vector<std::vector<int>> sEdgeIsects(nS), cEdgeIsects(nC);
    for (int i = 0; i < (int)isects.size(); i++) {
        sEdgeIsects[isects[i].sEdge].push_back(i);
        cEdgeIsects[isects[i].cEdge].push_back(i);
    }
    for (auto& v : sEdgeIsects)
        std::sort(v.begin(), v.end(), [&](int a, int b) { return isects[a].tS < isects[b].tS; });
    for (auto& v : cEdgeIsects)
        std::sort(v.begin(), v.end(), [&](int a, int b) { return isects[a].tC < isects[b].tC; });

    // Insere interseções na lista do Sujeito, em ordem ao longo de cada aresta.
    // Armazena ponteiros para os nós inseridos para conectar os portais depois.
    std::vector<Vertex*> isectSPtr(isects.size()), isectCPtr(isects.size());
    {
        auto it = sList.begin();
        for (int si = 0; si < nS; si++) {
            ++it; // Avança para a posição logo após o vértice original da aresta si
            for (int iIdx : sEdgeIsects[si]) {
                auto inserted = sList.insert(it, Vertex(isects[iIdx].pos));
                inserted->isIntersection = true;
                isectSPtr[iIdx] = &(*inserted);
            }
        }
    }

    // Insere interseções na lista do Clipper, analogamente
    {
        auto it = cList.begin();
        for (int ci = 0; ci < nC; ci++) {
            ++it;
            for (int iIdx : cEdgeIsects[ci]) {
                auto inserted = cList.insert(it, Vertex(isects[iIdx].pos));
                inserted->isIntersection = true;
                isectCPtr[iIdx] = &(*inserted);
            }
        }
    }

    // Conecta vizinhos via ponteiros de memória (portais entre as duas listas)
    for (int i = 0; i < (int)isects.size(); i++) {
        isectSPtr[i]->neighbor = isectCPtr[i];
        isectCPtr[i]->neighbor = isectSPtr[i];
    }
}

// --- FASE 2: Classificação Entrada/Saída ---
// Percorre a lista do sujeito e, a partir do estado do primeiro vértice
// original (dentro ou fora do clipper), alterna as interseções entre
// Entry e Exit. Propriedade garantida para polígonos simples.

void classifyIntersections(PolygonList& sList,
                           const std::vector<glm::vec2>& clipPoly) {
    bool outside = true;
    for (auto& v : sList)
        if (!v.isIntersection) { outside = !isInside(v.pos, clipPoly); break; }

    bool nextIsEntry = outside;
    for (auto& v : sList) {
        if (v.isIntersection) {
            v.entry = nextIsEntry;
            nextIsEntry = !nextIsEntry;
        }
    }
}

// --- FASE 3: Travessia (The Walk) ---
// Para cada interseção de Entrada não visitada na lista do sujeito:
//   1. Percorre S adicionando vértices até encontrar uma Saída.
//   2. Na Saída, salta (portal) para a lista C via ponteiro 'neighbor'.
//   3. Percorre C até encontrar a próxima interseção.
//   4. Na interseção de C, se neighbor aponta para o início → fecha polígono.
//      Senão, salta de volta para S via ponteiro e repete.

// Busca o iterador de um nó na lista a partir do ponteiro de memória.
// Vantagem do std::list: ponteiros para elementos permanecem estáveis.
PolygonList::iterator findByPtr(PolygonList& L, Vertex* ptr) {
    for (auto it = L.begin(); it != L.end(); ++it)
        if (&(*it) == ptr) return it;
    return L.end();
}

std::vector<std::vector<glm::vec2>> performTheWalk(
    PolygonList& sList,
    PolygonList& cList)
{
    std::vector<std::vector<glm::vec2>> results;
    int totalSize = (int)sList.size() + (int)cList.size();

    for (auto startIt = sList.begin(); startIt != sList.end(); ++startIt) {
        if (!startIt->isIntersection || !startIt->entry || startIt->visited)
            continue;

        std::vector<glm::vec2> polygon;
        Vertex* startPtr = &(*startIt); // Ponteiro para o nó de início
        bool walkingSubject = true;
        int safety = totalSize;

        // Começa a travessia no nó de Entrada em S
        auto sIt = startIt;
        PolygonList::iterator cIt; // Será definido ao pular para C

        while (safety-- > 0) {
            if (walkingSubject) {
                // Marca e adiciona o vértice de Entrada atual
                sIt->visited = true;
                polygon.push_back(sIt->pos);
                sIt = circularNext(sList, sIt);

                // Caminha na lista S até a próxima interseção (Saída)
                while (!sIt->isIntersection && safety-- > 0) {
                    polygon.push_back(sIt->pos);
                    sIt = circularNext(sList, sIt);
                }

                // Interseção de Saída: adiciona e pula para C via ponteiro
                sIt->visited = true;
                polygon.push_back(sIt->pos);
                Vertex* cNeighbor = sIt->neighbor;
                cIt = findByPtr(cList, cNeighbor);
                cIt->visited = true;
                cIt = circularNext(cList, cIt);
                walkingSubject = false;
            } else {
                // Percorre a lista C até a próxima interseção
                while (!cIt->isIntersection && safety-- > 0) {
                    polygon.push_back(cIt->pos);
                    cIt = circularNext(cList, cIt);
                }

                // Interseção em C: verifica se voltamos ao início via ponteiro
                cIt->visited = true;
                Vertex* sNeighbor = cIt->neighbor;
                if (sNeighbor == startPtr) break; // Polígono fechado!

                // Salta de volta para S via ponteiro e continua
                sIt = findByPtr(sList, sNeighbor);
                walkingSubject = true;
            }
        }

        if (polygon.size() >= 3)
            results.push_back(polygon);
    }
    return results;
}

// --- Shaders GLSL 330 Core ---

const char* vertexShaderSrc =
    "#version 330 core\n"
    "layout(location = 0) in vec2 aPos;\n"
    "void main() {\n"
    "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "}\n";

const char* fragmentShaderSrc =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec3 uColor;\n"
    "void main() {\n"
    "    FragColor = vec4(uColor, 1.0);\n"
    "}\n";

// --- Programa Principal ---

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600,
        "Exercício 20 - Weiler-Atherton (Polígono com Buraco)", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // --- Geometria de Teste ---
    // Sujeito: retângulo externo (CCW) com buraco retangular interno (CW),
    // conectados por uma ponte (bridge) que permite representar o contorno
    // como um único polígono, conforme exigido pelo Weiler-Atherton.
    //
    //   (-0.8, 0.8)----------(0.8, 0.8)   Contorno externo (anti-horário)
    //       |  (-0.3,0.3)---(0.3,0.3) |
    //       |  |    BURACO         |   |   Buraco (horário)
    //       |  (-0.3,-0.1)-(0.3,-0.1)  |
    //   (-0.8,-0.6)----------(0.8,-0.6)
    //
    // A ponte conecta (-0.8,0.8) ↔ (-0.3,0.3), criando um corte
    // que inverte a orientação local sem cruzar o clipper.

    std::vector<glm::vec2> outerPts = {
        {-0.8f, -0.6f}, {0.8f, -0.6f}, {0.8f, 0.8f}, {-0.8f, 0.8f}
    };
    std::vector<glm::vec2> holePts = {
        {-0.3f, 0.3f}, {0.3f, 0.3f}, {0.3f, -0.1f}, {-0.3f, -0.1f}
    };

    // Sujeito completo: externo CCW → ponte → buraco CW → ponte de volta
    std::vector<glm::vec2> sPts = {
        {-0.8f, -0.6f}, {0.8f, -0.6f}, {0.8f, 0.8f}, {-0.8f, 0.8f},
        {-0.3f,  0.3f},  // ponte para o buraco
        {0.3f,  0.3f}, {0.3f, -0.1f}, {-0.3f, -0.1f},  // buraco CW
        {-0.3f,  0.3f}, {-0.8f, 0.8f}   // ponte de volta
    };

    // Clipper (anti-horário): retângulo deslocado para a direita
    std::vector<glm::vec2> cPts = {
        {-0.1f, -0.8f}, {0.9f, -0.8f}, {0.9f, 0.5f}, {-0.1f, 0.5f}
    };

    // --- Execução do Algoritmo ---
    PolygonList sList, cList;
    buildAugmentedLists(sPts, cPts, sList, cList);
    classifyIntersections(sList, cPts);
    auto clippedPolygons = performTheWalk(sList, cList);

    // Diagnóstico no terminal
    int numIsects = 0;
    for (auto& v : sList) if (v.isIntersection) numIsects++;
    std::cout << "Interseções encontradas: " << numIsects << std::endl;
    for (size_t p = 0; p < clippedPolygons.size(); p++) {
        std::cout << "Polígono recortado #" << p << " ("
                  << clippedPolygons[p].size() << " vértices):" << std::endl;
        for (auto& v : clippedPolygons[p])
            std::cout << "  (" << v.x << ", " << v.y << ")" << std::endl;
    }

    // --- Configuração OpenGL ---
    GLuint prog = glCreateProgram();
    GLuint vS = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vS, 1, &vertexShaderSrc, NULL); glCompileShader(vS);
    GLuint fS = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fS, 1, &fragmentShaderSrc, NULL); glCompileShader(fS);
    glAttachShader(prog, vS); glAttachShader(prog, fS);
    glLinkProgram(prog);
    glDeleteShader(vS); glDeleteShader(fS);

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);

    // --- Loop de Renderização ---
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(prog);
        GLint colorLoc = glGetUniformLocation(prog, "uColor");

        // Sujeito - contorno externo (azul)
        glLineWidth(2.0f);
        glUniform3f(colorLoc, 0.0f, 0.4f, 1.0f);
        glBufferData(GL_ARRAY_BUFFER, outerPts.size() * sizeof(glm::vec2),
                     outerPts.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)outerPts.size());

        // Sujeito - buraco (azul, linha fina para distinguir)
        glLineWidth(1.0f);
        glBufferData(GL_ARRAY_BUFFER, holePts.size() * sizeof(glm::vec2),
                     holePts.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)holePts.size());

        // Clipper (verde)
        glLineWidth(2.0f);
        glUniform3f(colorLoc, 0.0f, 1.0f, 0.3f);
        glBufferData(GL_ARRAY_BUFFER, cPts.size() * sizeof(glm::vec2),
                     cPts.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)cPts.size());

        // Resultado do recorte (amarelo, linha grossa + pontos nos vértices)
        for (auto& poly : clippedPolygons) {
            glLineWidth(4.0f);
            glUniform3f(colorLoc, 1.0f, 0.9f, 0.0f);
            glBufferData(GL_ARRAY_BUFFER, poly.size() * sizeof(glm::vec2),
                         poly.data(), GL_DYNAMIC_DRAW);
            glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)poly.size());

            glPointSize(10.0f);
            glDrawArrays(GL_POINTS, 0, (GLsizei)poly.size());
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(prog);
    glfwTerminate();
    return 0;
}