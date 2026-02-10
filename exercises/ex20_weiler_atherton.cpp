#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
#include <map>

// --- Estruturas de Dados ---

struct Vertex {
    glm::vec2 pos;
    bool isIntersection = false;
    bool entry = false;      // true = Entrada, false = Saída
    bool visited = false;
    Vertex* neighbor = nullptr; // Ponteiro para o "irmão" na outra lista

    Vertex() : pos(0,0) {}
    Vertex(glm::vec2 p) : pos(p) {}
};

using Contour = std::list<Vertex>;

struct InterRecord {
    Contour::iterator s_it;
    Contour::iterator c_it;
    double tS; // Parâmetro na aresta do sujeito [0,1]
    double tC; // Parâmetro na aresta de corte [0,1]
    glm::vec2 p;
};

// --- Funções Matemáticas e Utilitários ---

static bool segIntersectParam(const glm::vec2 &p, const glm::vec2 &r,
                              const glm::vec2 &q, const glm::vec2 &s,
                              double &tOut, double &uOut) {
    float rxs = r.x * s.y - r.y * s.x;
    if (std::abs(rxs) < 1e-9f) return false; // Paralelas
    
    glm::vec2 qp = q - p;
    tOut = (qp.x * s.y - qp.y * s.x) / rxs;
    uOut = (qp.x * r.y - qp.y * r.x) / rxs;
    return tOut >= 0 && tOut <= 1 && uOut >= 0 && uOut <= 1;
}

static bool pointInPolygon(const glm::vec2 &pt, const Contour &poly) {
    bool inside = false;
    if (poly.empty()) return false;
    for (auto it = poly.begin(); it != poly.end(); ++it) {
        auto next = std::next(it) == poly.end() ? poly.begin() : std::next(it);
        if (((it->pos.y > pt.y) != (next->pos.y > pt.y)) &&
            (pt.x < (next->pos.x - it->pos.x) * (pt.y - it->pos.y) / (next->pos.y - it->pos.y + 1e-12f) + it->pos.x))
            inside = !inside;
    }
    return inside;
}

// --- Fases do Weiler-Atherton ---

// FASE 1: Inserir interseções ordenadamente
void computeAndInsertIntersections(std::vector<Contour*> &subjects, Contour &clip) {
    std::vector<InterRecord> records;
    for (auto scontPtr : subjects) {
        Contour &S = *scontPtr;
        for (auto sit = S.begin(); sit != S.end(); ++sit) {
            auto snext = std::next(sit) == S.end() ? S.begin() : std::next(sit);
            for (auto cit = clip.begin(); cit != clip.end(); ++cit) {
                auto cnext = std::next(cit) == clip.end() ? clip.begin() : std::next(cit);
                double t, u;
                if (segIntersectParam(sit->pos, snext->pos - sit->pos, cit->pos, cnext->pos - cit->pos, t, u)) {
                    glm::vec2 ip = sit->pos + (float)t * (snext->pos - sit->pos);
                    records.push_back({sit, cit, t, u, ip});
                }
            }
        }
    }

    // Inserção no Sujeito
    for (auto rec : records) {
        Vertex v(rec.p); v.isIntersection = true;
        // Simplificação: apenas insere. No real, deve ordenar por tS se houver múltiplas por aresta.
        auto insertedS = std::next(rec.s_it);
        rec.s_it = subjects[0]->insert(insertedS, v); // Assume 1o contorno para brevidade
        
        Vertex vc(rec.p); vc.isIntersection = true;
        auto insertedC = std::next(rec.c_it);
        rec.c_it = clip.insert(insertedC, vc);
        
        rec.s_it->neighbor = &(*rec.c_it);
        rec.c_it->neighbor = &(*rec.s_it);
    }
}

// FASE 2: Classificar Entrada/Saída
void classifyIntersections(Contour &sList, const Contour &clip) {
    for (auto& v : sList) {
        if (v.isIntersection) {
            // Se o ponto está entrando no polígono de corte
            v.entry = pointInPolygon(v.pos + glm::vec2(0.001f), clip); 
        }
    }
}

// --- Shaders e Renderização ---

const char* vs_src = "#version 330 core\nlayout(location=0) in vec2 p; void main(){gl_Position=vec4(p,0,1);}\0";
const char* fs_src = "#version 330 core\nout vec4 f; uniform vec3 c; void main(){f=vec4(c,1);}\0";

void drawOutline(const std::vector<glm::vec2>& pts, glm::vec3 color, GLint colorLoc) {
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao); glGenBuffers(1, &vbo);
    glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, pts.size() * sizeof(glm::vec2), pts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)pts.size());
    glDeleteBuffers(1, &vbo); glDeleteVertexArrays(1, &vao);
}

int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(800, 600, "Weiler-Atherton Corrigido", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // Definir polígonos iniciais
    std::vector<glm::vec2> sPts = {{-0.6f, 0.4f}, {0.2f, 0.4f}, {0.2f, -0.4f}, {-0.6f, -0.4f}};
    std::vector<glm::vec2> cPts = {{-0.3f, 0.2f}, {0.5f, 0.2f}, {0.5f, -0.6f}, {-0.3f, -0.6f}};

    Contour sList, cList;
    for(auto p : sPts) sList.push_back(Vertex(p));
    for(auto p : cPts) cList.push_back(Vertex(p));
    
    std::vector<Contour*> subjects = {&sList};
    computeAndInsertIntersections(subjects, cList);
    classifyIntersections(sList, cList);

    // Shaders
    GLuint vs = glCreateShader(GL_VERTEX_SHADER); glShaderSource(vs, 1, &vs_src, NULL); glCompileShader(vs);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER); glShaderSource(fs, 1, &fs_src, NULL); glCompileShader(fs);
    GLuint prog = glCreateProgram(); glAttachShader(prog, vs); glAttachShader(prog, fs); glLinkProgram(prog);
    GLint cLoc = glGetUniformLocation(prog, "c");

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(prog);

        drawOutline(sPts, {0, 0.5f, 1}, cLoc); // Sujeito Azul
        drawOutline(cPts, {0, 1, 0}, cLoc);    // Corte Verde

        // Desenhar pontos de interseção em amarelo para provar que a Fase 1 funcionou
        std::vector<glm::vec2> inters;
        for(auto& v : sList) if(v.isIntersection) inters.push_back(v.pos);
        if(!inters.empty()) {
            glPointSize(10.0f);
            glUniform3f(cLoc, 1, 1, 0);
            GLuint vao, vbo; glGenVertexArrays(1, &vao); glGenBuffers(1, &vbo);
            glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, inters.size()*8, inters.data(), GL_STATIC_DRAW);
            glEnableVertexAttribArray(0); glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8, 0);
            glDrawArrays(GL_POINTS, 0, (GLsizei)inters.size());
            glDeleteBuffers(1, &vbo); glDeleteVertexArrays(1, &vao);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}