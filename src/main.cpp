/*
 * Universidade Federal de Ouro Preto - UFOP
 * Departamento de Computação - DECOM
 * Disciplina: BCC327 - Computação Gráfica (2025.2)
 * Professor: Rafael Bonfim
 * Trabalho Prático: Visualizador de Árvores Arteriais (CCO)
 * Arquivo: main.cpp
 * Autor: Mateus Honorato
 * Data: Fevereiro/2026
 * Descrição:
 * Inicialização da aplicação, loop principal e callbacks GLFW.
 * Créditos:
 * Estrutura de inicialização da janela (GLFW/GLAD) e integração
 * da interface gráfica adaptadas dos exemplos de código fornecidos
 * pela biblioteca Dear ImGui (Omar Cornut) e tutoriais do LearnOpenGL.com.
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "AnimationController.hpp"
#include "MenuController.hpp"
#include "ClippingUtils.hpp"
#include "VtkReader.hpp"
#include "Shader.hpp"
#include "TreeRenderer.hpp"
#include "SceneContext.hpp"
#include "ScreenshotUtils.hpp"
#include "Camera.hpp"
#include "PickingUtils.hpp"
#include "ArterialTree.hpp"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const int OPENGL_MAJOR_VERSION = 3;
const int OPENGL_MINOR_VERSION = 3;
const float ROTATION_ANGLE = -90.0f;
const glm::vec4 CLEAR_COLOR = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
const float ORTHO_NEAR_PLANE = 0.1f;
const float ORTHO_FAR_PLANE = 100.0f;
const float PERSPECTIVE_FOV = 45.0f;
const float PERSPECTIVE_NEAR_PLANE = 0.1f;
const float PERSPECTIVE_FAR_PLANE = 100.0f;
const float HIT_RADIUS_MULTIPLIER = 1.1f;
const float MIN_HIT_RADIUS = 0.0001f;

struct AppContext
{
    Camera camera;
    AnimationController animCtrl;
    ArterialTree tree;
    SceneContext sceneCtx;
    glm::mat4 view;
    glm::mat4 projection;
};

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureMouse)
        return;

    AppContext *context = static_cast<AppContext *>(glfwGetWindowUserPointer(window));
    if (!context)
        return;

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // --- LÓGICA DE PICKING (Botão Esquerdo) ---
    // Se Ctrl NÃO está pressionado, faz picking normalmente
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !(mods & GLFW_MOD_CONTROL))
    {
        // 1. Configuração DPI
        int winWidth, winHeight;
        glfwGetWindowSize(window, &winWidth, &winHeight);
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

        float xScale = (float)fbWidth / (float)winWidth;
        float yScale = (float)fbHeight / (float)winHeight;
        double mouseX_FB = xpos * xScale;
        double mouseY_FB = ypos * yScale;

        // 2. Pegar View Matrix Atualizada
        glm::mat4 currentView = context->camera.getViewMatrix();

        // 3. CALCULAR RAIO (Origem + Direção)
        // Usamos a nova função que lida com Ortho e Perspective automaticamente
        glm::vec3 rayOrigin, rayDir;
        PickingUtils::getRayFromMouse(mouseX_FB, mouseY_FB, fbWidth, fbHeight, currentView, context->projection, rayOrigin, rayDir);

        // 4. Matriz Model (-90 graus em 3D)
        glm::mat4 model = glm::mat4(1.0f);
        if (context->animCtrl.getCurrentMode() == AnimationController::Mode3D)
        {
            model = glm::rotate(model, glm::radians(ROTATION_ANGLE), glm::vec3(1.0f, 0.0f, 0.0f));
        }

        int closestIdx = -1;
        float closestDist = std::numeric_limits<float>::max();

        for (size_t i = 0; i < context->tree.segments.size(); ++i)
        {
            const auto &seg = context->tree.segments[i];
            glm::vec3 a = context->tree.nodes[seg.indexA].position;
            glm::vec3 b = context->tree.nodes[seg.indexB].position;
            // Clipping check
            if (context->animCtrl.clipping.enabled)
            {
                glm::vec3 tempA = a, tempB = b;
                if (!ClippingUtils::clipSegment(tempA, tempB, context->animCtrl.clipping.min, context->animCtrl.clipping.max))
                {
                    continue;
                }
            }
            glm::vec4 localA = glm::vec4(a, 1.0f);
            glm::vec4 localB = glm::vec4(b, 1.0f);
            glm::vec3 worldA = glm::vec3(model * localA);
            glm::vec3 worldB = glm::vec3(model * localB);
            float hitRadius = std::max(seg.radius * context->animCtrl.radiusScale * HIT_RADIUS_MULTIPLIER, MIN_HIT_RADIUS);
            float outDist;
            if (PickingUtils::rayIntersectsSegment(rayOrigin, rayDir, worldA, worldB, hitRadius, outDist))
            {
                if (outDist < closestDist)
                {
                    closestDist = outDist;
                    closestIdx = static_cast<int>(i);
                }
            }
        }

        context->animCtrl.selectSegment(closestIdx, context->tree);
    }

    // Se Ctrl está pressionado E botão esquerdo, trata como pan (mão)
    // (aciona o pan do botão direito, mas com o esquerdo+Ctrl)
    if (button == GLFW_MOUSE_BUTTON_LEFT && (mods & GLFW_MOD_CONTROL))
    {
        // Simula botão direito para Camera
        context->camera.processMouseButton(GLFW_MOUSE_BUTTON_RIGHT, action, xpos, ypos);
        return;
    }

    context->camera.processMouseButton(button, action, xpos, ypos);
}

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    AppContext *context = static_cast<AppContext *>(glfwGetWindowUserPointer(window));
    if (!context)
        return;
    context->camera.processMouseMovement(xpos, ypos);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    if (ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureMouse)
        return;

    AppContext *context = static_cast<AppContext *>(glfwGetWindowUserPointer(window));
    if (!context)
        return;
    context->camera.processMouseScroll(static_cast<float>(yoffset));
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main(int argc, char *argv[])
{
    // Obtém o caminho do executável
    std::string exePath = argv[0];
    std::string exeDir = exePath.substr(0, exePath.find_last_of("/\\"));

    // Inicialização do GLFW
    if (!glfwInit())
        return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_MAJOR_VERSION);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_MINOR_VERSION);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Arterial Tree Visualization", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    AppContext context;
    glfwSetWindowUserPointer(window, &context);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Inicialização do GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        return -1;

    // Cursor para indicar pan
    GLFWcursor *handCursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    GLFWcursor *arrowCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);

    // Inicialização do ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    // io.FontGlobalScale = 1.5f;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Configuração OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Constrói os caminhos para os shaders
    std::string vertexShaderPath = exeDir + "/../shaders/vertex.glsl";
    std::string fragmentShaderPath = exeDir + "/../shaders/fragment.glsl";
    std::string lineVertexShaderPath = exeDir + "/../shaders/line_vertex.glsl";
    std::string lineFragmentShaderPath = exeDir + "/../shaders/line_fragment.glsl";

    // Objetos do Domínio
    Shader shader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
    Shader lineShader(lineVertexShaderPath.c_str(), lineFragmentShaderPath.c_str());
    TreeRenderer renderer;
    MenuController menuCtrl;
    context.sceneCtx.init();
    context.view = glm::mat4(1.0f);
    context.projection = glm::mat4(1.0f);

    double lastTime = glfwGetTime();

    // Loop Principal
    while (!glfwWindowShouldClose(window))
    {
        // Lógica de captura de tela
        bool isSnapshot = context.animCtrl.isScreenshotRequested();

        // Limpar buffer de cor (transparente para screenshot, normal caso contrário)
        if (isSnapshot)
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        else
            glClearColor(CLEAR_COLOR.r, CLEAR_COLOR.g, CLEAR_COLOR.b, CLEAR_COLOR.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Lógica de animação
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;
        processInput(window);
        context.animCtrl.update(deltaTime, context.tree, renderer);

        // Atualiza razão de aspecto e matrizes de view/projection
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        float aspect = (height > 0) ? (float)width / (float)height : 1.0f;
        context.view = context.camera.getViewMatrix();
        if (context.animCtrl.useOrthographic)
        {
            float orthoHeight = glm::length(context.camera.getPosition());
            orthoHeight = std::max(orthoHeight, 0.1f);
            float orthoWidth = orthoHeight * aspect;
            context.projection = glm::ortho(-orthoWidth, orthoWidth, -orthoHeight, orthoHeight, ORTHO_NEAR_PLANE, ORTHO_FAR_PLANE);
        }
        else
        {
            context.projection = glm::perspective(glm::radians(PERSPECTIVE_FOV), aspect, PERSPECTIVE_NEAR_PLANE, PERSPECTIVE_FAR_PLANE);
        }

        // Cursor para indicar pan
        bool isPanning = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
        glfwSetCursor(window, isPanning ? handCursor : arrowCursor);

        // Atualização visual quando necessário
        if (context.animCtrl.isVisualDirty())
        {
            if (context.animCtrl.getCurrentMode() == AnimationController::ModeWireframe)
            {
                renderer.initWireframe(context.tree.nodes, context.tree.segments,
                                       context.animCtrl.clipping.enabled,
                                       context.animCtrl.clipping.min,
                                       context.animCtrl.clipping.max);
            }
            else
            {
                renderer.init(context.tree,
                              context.animCtrl.radiusScale,
                              context.animCtrl.showSpheres,
                              context.animCtrl.clipping.enabled,
                              context.animCtrl.clipping.min,
                              context.animCtrl.clipping.max);
            }
            context.animCtrl.resetVisualDirty();
        }

        // Configurar uniforms do shader e desenhar a cena
        glm::mat4 model = glm::mat4(1.0f);
        if (context.animCtrl.getCurrentMode() == AnimationController::Mode3D)
        {
            model = glm::rotate(model, glm::radians(ROTATION_ANGLE), glm::vec3(1.0f, 0.0f, 0.0f));
        }
        shader.use();
        shader.setMat4("model", model);
        shader.setMat4("view", context.view);
        shader.setMat4("projection", context.projection);
        shader.setVec3("lightPos", glm::vec3(context.animCtrl.lightPos[0], context.animCtrl.lightPos[1], context.animCtrl.lightPos[2]));
        shader.setVec3("viewPos", context.camera.getPosition());
        shader.setFloat("alpha", context.animCtrl.transparency);
        shader.setInt("lightingMode", context.animCtrl.lightingMode);

        if (context.animCtrl.getCurrentMode() == AnimationController::ModeWireframe)
        {
            lineShader.use();
            lineShader.setMat4("model", model);
            lineShader.setMat4("view", context.view);
            lineShader.setMat4("projection", context.projection);
            lineShader.setInt("selectedSegmentID", context.animCtrl.getSelectedSegment());
            lineShader.setFloat("alpha", context.animCtrl.transparency);
            renderer.drawWireframe(lineShader, context.view, context.projection, model, context.animCtrl.lineWidth, context.animCtrl.getSelectedSegment());
        }
        else
        {
            renderer.draw(shader, context.view, context.projection, model, context.animCtrl.getSelectedSegment());
        }

        // Desenhar grade (grid) e gizmo
        if (context.animCtrl.showGrid)
        {
            context.sceneCtx.drawGrid(context.view, context.projection);
        }
        if (context.animCtrl.showGizmo)
        {
            context.sceneCtx.drawGizmo(context.view, context.projection, width, height);
        }

        // Renderizar UI (ocultar menu principal se for snapshot)
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // Atalhos globais de teclado (processados mesmo quando o menu principal está colapsado/oculto)
        // Respeita captura de teclado do ImGui para não interferir em widgets de texto.
        ImGuiIO &io = ImGui::GetIO();
        if (!io.WantCaptureKeyboard && ImGui::IsKeyPressed(ImGuiKey_Space))
        {
            context.animCtrl.togglePlay();
            context.animCtrl.m_visualDirty = true;
        }
        // Atalho: 'P' para capturar screenshot (respeita captura do ImGui)
        if (!io.WantCaptureKeyboard && ImGui::IsKeyPressed(ImGuiKey_P))
        {
            context.animCtrl.requestScreenshot();
        }
        menuCtrl.render(context.animCtrl, context.tree, renderer, isSnapshot);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Captura de screenshot (antes do swap)
        if (isSnapshot)
        {
            ScreenshotUtils::saveScreenshot(width, height);
            context.animCtrl.resetScreenshotRequest();
        }

        // Troca de buffers (swap) e poll de eventos
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Limpeza de recursos
    glfwDestroyCursor(handCursor);
    glfwDestroyCursor(arrowCursor);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}