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

// Instância global de câmera
Camera camera;

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Declarações externas para acesso
extern AnimationController animCtrl;
extern ArterialTree tree;
extern SceneContext sceneCtx;
extern glm::mat4 view;
extern glm::mat4 projection;

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureMouse)
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
        glm::mat4 currentView = camera.getViewMatrix();

        // 3. CALCULAR RAIO (Origem + Direção)
        // Usamos a nova função que lida com Ortho e Perspective automaticamente
        glm::vec3 rayOrigin, rayDir;
        PickingUtils::getRayFromMouse(mouseX_FB, mouseY_FB, fbWidth, fbHeight, currentView, projection, rayOrigin, rayDir);

        // 4. Matriz Model (-90 graus em 3D)
        glm::mat4 model = glm::mat4(1.0f);
        if (animCtrl.getCurrentMode() == AnimationController::Mode3D)
        {
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        }

        int closestIdx = -1;
        float closestDist = std::numeric_limits<float>::max();

        for (size_t i = 0; i < tree.segments.size(); ++i)
        {
            const auto &seg = tree.segments[i];
            glm::vec3 a = tree.nodes[seg.indexA].position;
            glm::vec3 b = tree.nodes[seg.indexB].position;
            // Clipping check
            if (animCtrl.clipping.enabled)
            {
                glm::vec3 tempA = a, tempB = b;
                if (!ClippingUtils::clipSegment(tempA, tempB, animCtrl.clipping.min, animCtrl.clipping.max))
                {
                    continue;
                }
            }
            glm::vec4 localA = glm::vec4(a, 1.0f);
            glm::vec4 localB = glm::vec4(b, 1.0f);
            glm::vec3 worldA = glm::vec3(model * localA);
            glm::vec3 worldB = glm::vec3(model * localB);
            float hitRadius = std::max(seg.radius * animCtrl.radiusScale * 1.1f, 0.0001f);
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

        animCtrl.selectSegment(closestIdx, tree);
    }

    // Se Ctrl está pressionado E botão esquerdo, trata como pan (mão)
    // (aciona o pan do botão direito, mas com o esquerdo+Ctrl)
    if (button == GLFW_MOUSE_BUTTON_LEFT && (mods & GLFW_MOD_CONTROL))
    {
        // Simula botão direito para Camera
        camera.processMouseButton(GLFW_MOUSE_BUTTON_RIGHT, action, xpos, ypos);
        return;
    }

    camera.processMouseButton(button, action, xpos, ypos);
}

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    camera.processMouseMovement(xpos, ypos);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    if (ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureMouse)
        return;
    camera.processMouseScroll(static_cast<float>(yoffset));
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// Variáveis globais usadas pelos callbacks (picking, UI, etc.)
AnimationController animCtrl;
ArterialTree tree;
SceneContext sceneCtx;
glm::mat4 view;
glm::mat4 projection;

int main()
{
    // Inicialização do GLFW
    if (!glfwInit())
        return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(1280, 720, "Arterial Tree Visualization", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

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

    // Objetos do Domínio

    Shader shader("../shaders/vertex.glsl", "../shaders/fragment.glsl");
    Shader lineShader("../shaders/line_vertex.glsl", "../shaders/line_fragment.glsl");
    TreeRenderer renderer;
    MenuController menuCtrl;
    sceneCtx.init();
    view = glm::mat4(1.0f);
    projection = glm::mat4(1.0f);

    double lastTime = glfwGetTime();

    // Loop Principal

    while (!glfwWindowShouldClose(window))
    {
        // Lógica de captura de tela
        bool isSnapshot = animCtrl.isScreenshotRequested();

        // Limpar buffer de cor (transparente para screenshot, normal caso contrário)
        if (isSnapshot)
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        else
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Lógica de animação
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;
        processInput(window);
        animCtrl.update(deltaTime, tree, renderer);

        // Atualiza razão de aspecto e matrizes de view/projection
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        float aspect = (height > 0) ? (float)width / (float)height : 1.0f;
        view = camera.getViewMatrix();
        if (animCtrl.useOrthographic)
        {
            float orthoHeight = glm::length(camera.getPosition());
            orthoHeight = std::max(orthoHeight, 0.1f);
            float orthoWidth = orthoHeight * aspect;
            projection = glm::ortho(-orthoWidth, orthoWidth, -orthoHeight, orthoHeight, 0.1f, 100.0f);
        }
        else
        {
            projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
        }

        // Cursor para indicar pan
        bool isPanning = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
        glfwSetCursor(window, isPanning ? handCursor : arrowCursor);

        // Atualização visual quando necessário
        if (animCtrl.isVisualDirty())
        {
            if (animCtrl.getCurrentMode() == AnimationController::ModeWireframe)
            {
                renderer.initWireframe(tree.nodes, tree.segments,
                                       animCtrl.clipping.enabled,
                                       animCtrl.clipping.min,
                                       animCtrl.clipping.max);
            }
            else
            {
                renderer.init(tree,
                              animCtrl.radiusScale,
                              animCtrl.showSpheres,
                              animCtrl.clipping.enabled,
                              animCtrl.clipping.min,
                              animCtrl.clipping.max);
            }
            animCtrl.resetVisualDirty();
        }

        // Configurar uniforms do shader e desenhar a cena
        glm::mat4 model = glm::mat4(1.0f);
        if (animCtrl.getCurrentMode() == AnimationController::Mode3D)
        {
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        }
        shader.use();
        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setVec3("lightPos", glm::vec3(animCtrl.lightPos[0], animCtrl.lightPos[1], animCtrl.lightPos[2]));
        shader.setVec3("viewPos", camera.getPosition());
        shader.setFloat("alpha", animCtrl.transparency);
        shader.setInt("lightingMode", animCtrl.lightingMode);

        if (animCtrl.getCurrentMode() == AnimationController::ModeWireframe)
        {
            lineShader.use();
            lineShader.setMat4("model", model);
            lineShader.setMat4("view", view);
            lineShader.setMat4("projection", projection);
            lineShader.setInt("selectedSegmentID", animCtrl.getSelectedSegment());
            lineShader.setFloat("alpha", animCtrl.transparency);
            renderer.drawWireframe(lineShader, view, projection, model, animCtrl.lineWidth, animCtrl.getSelectedSegment());
        }
        else
        {
            renderer.draw(shader, view, projection, model, animCtrl.getSelectedSegment());
        }

        // Desenhar grade (grid) e gizmo
        if (animCtrl.showGrid)
        {
            sceneCtx.drawGrid(view, projection);
        }
        if (animCtrl.showGizmo)
        {
            sceneCtx.drawGizmo(view, projection, width, height);
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
            animCtrl.togglePlay();
            animCtrl.m_visualDirty = true;
        }
        // Atalho: 'P' para capturar screenshot (respeita captura do ImGui)
        if (!io.WantCaptureKeyboard && ImGui::IsKeyPressed(ImGuiKey_P))
        {
            animCtrl.requestScreenshot();
        }
        menuCtrl.render(animCtrl, tree, renderer, isSnapshot);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Captura de screenshot (antes do swap)
        if (isSnapshot)
        {
            ScreenshotUtils::saveScreenshot(width, height);
            animCtrl.resetScreenshotRequest();
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