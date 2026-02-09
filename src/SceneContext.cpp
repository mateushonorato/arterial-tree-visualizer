#include "SceneContext.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <array>
#include <iostream>

static const char* gridVertexShader = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
uniform mat4 view;
uniform mat4 projection;
out vec3 vColor;
void main() {
    vColor = aColor;
    gl_Position = projection * view * vec4(aPos, 1.0);
}
)";

static const char* gridFragmentShader = R"(
#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(vColor, 1.0);
}
)";

SceneContext::SceneContext() {}
SceneContext::~SceneContext() {
    if (gridVAO) glDeleteVertexArrays(1, &gridVAO);
    if (gridVBO) glDeleteBuffers(1, &gridVBO);
    if (gizmoVAO) glDeleteVertexArrays(1, &gizmoVAO);
    if (gizmoVBO) glDeleteBuffers(1, &gizmoVBO);
    if (shaderProgram) glDeleteProgram(shaderProgram);
}

void SceneContext::init() {
    shaderProgram = compileShader(gridVertexShader, gridFragmentShader);
    createGridLines();
    createGizmoAxes();
}

void SceneContext::createGridLines() {
    std::vector<float> gridVerts;
    float min = -10.0f, max = 10.0f;
    int N = 21; // 21 lines per axis
    float colorMajor[3] = {0.5f, 0.5f, 0.5f};
    float colorMinor[3] = {0.3f, 0.3f, 0.3f};
    for (int i = 0; i < N; ++i) {
        float x = min + (max - min) * i / (N - 1);
        // X lines (parallel to Z)
        gridVerts.insert(gridVerts.end(), {x, 0, min, colorMinor[0], colorMinor[1], colorMinor[2]});
        gridVerts.insert(gridVerts.end(), {x, 0, max, colorMinor[0], colorMinor[1], colorMinor[2]});
        // Z lines (parallel to X)
        gridVerts.insert(gridVerts.end(), {min, 0, x, colorMinor[0], colorMinor[1], colorMinor[2]});
        gridVerts.insert(gridVerts.end(), {max, 0, x, colorMinor[0], colorMinor[1], colorMinor[2]});
    }
    // Major axes (darker)
    gridVerts.insert(gridVerts.end(), {min, 0, 0, colorMajor[0], colorMajor[1], colorMajor[2]});
    gridVerts.insert(gridVerts.end(), {max, 0, 0, colorMajor[0], colorMajor[1], colorMajor[2]});
    gridVerts.insert(gridVerts.end(), {0, 0, min, colorMajor[0], colorMajor[1], colorMajor[2]});
    gridVerts.insert(gridVerts.end(), {0, 0, max, colorMajor[0], colorMajor[1], colorMajor[2]});
    gridLineCount = (int)(gridVerts.size() / 6);
    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);
    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, gridVerts.size() * sizeof(float), gridVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void SceneContext::createGizmoAxes() {
    float axes[] = {
        // X axis (red)
        0,0,0, 1,0,0,   1,0,0, 1,0,0,
        // Y axis (green)
        0,0,0, 0,1,0,   0,1,0, 0,1,0,
        // Z axis (blue)
        0,0,0, 0,0,1,   0,0,1, 0,0,1
    };
    glGenVertexArrays(1, &gizmoVAO);
    glGenBuffers(1, &gizmoVBO);
    glBindVertexArray(gizmoVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gizmoVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

GLuint SceneContext::compileShader(const char* vsrc, const char* fsrc) {
    GLint success;
    char infoLog[512];

    // Vertex Shader
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsrc, nullptr);
    glCompileShader(vs);
    // Verificação de Erro
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vs, 512, nullptr, infoLog);
        std::cerr << "ERRO::SCENE::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Fragment Shader
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsrc, nullptr);
    glCompileShader(fs);
    // Verificação de Erro
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fs, 512, nullptr, infoLog);
        std::cerr << "ERRO::SCENE::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Program Link
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    // Verificação de Erro de Linkagem
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(prog, 512, nullptr, infoLog);
        std::cerr << "ERRO::SCENE::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

void SceneContext::drawGrid(const glm::mat4& view, const glm::mat4& projection) {
    glUseProgram(shaderProgram);
    glBindVertexArray(gridVAO);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    glDrawArrays(GL_LINES, 0, gridLineCount);
    glBindVertexArray(0);
    glUseProgram(0);
}

void SceneContext::drawGizmo(const glm::mat4& view, const glm::mat4& projection, int screenWidth, int screenHeight) {
    // 1. Salvar Viewport atual (da tela cheia)
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    // 2. Definir área do Gizmo (Quadrado de 100x100 no canto inferior direito)
    int size = 100;
    int x = screenWidth - size - 10; // 10px de margem da direita
    int y = 10;                      // 10px de margem de baixo
    glViewport(x, y, size, size);

    // 3. Preparar Matriz de View (Apenas Rotação)
    // Copiamos a view da câmera mas zeramos a coluna de translação.
    // Assim o gizmo gira junto com a câmera, mas fica fixo na origem do viewport dele.
    glm::mat4 gizmoView = view;
    gizmoView[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); 

    // 4. Preparar Matriz de Projeção (Fixa e Quadrada)
    // Usamos aspect ratio 1.0f para o gizmo não distorcer.
    // Movemos a câmera virtual um pouco para trás (-3.0f) para ver os eixos.
    glm::mat4 gizmoProj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 10.0f);
    gizmoView = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f)) * gizmoView;

    // 5. Configurar Shader e Estados
    glUseProgram(shaderProgram);
    
    // [TRUQUE] Desabilita teste de profundidade para desenhar "na frente" de tudo
    glDisable(GL_DEPTH_TEST);

    unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
    unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");
    
    // Passa as matrizes ESPECÍFICAS do Gizmo (não as da cena principal)
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &gizmoView[0][0]);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &gizmoProj[0][0]);

    // 6. Desenhar Eixos
    glBindVertexArray(gizmoVAO);
    glDrawArrays(GL_LINES, 0, 6); // 3 eixos * 2 vértices (RGB)
    glBindVertexArray(0);

    // 7. Restaurar Estados Originais
    // É crucial reativar o Depth Test e voltar o Viewport ao normal
    // senão o resto do programa quebra no próximo frame.
    glEnable(GL_DEPTH_TEST);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    glUseProgram(0);
}