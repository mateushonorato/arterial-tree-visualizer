#include "ClippingUtils.hpp"
#include "MenuController.hpp"
#include "AnimationController.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "VtkReader.hpp"
#include "Shader.hpp"
#include "TreeRenderer.hpp"
#include "SceneContext.hpp"
#include "ScreenshotUtils.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Camera.hpp"
#include "PickingUtils.hpp"


// Global camera instance
Camera camera;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Forward declarations for access
extern AnimationController animCtrl;
extern ArterialTree tree;
extern SceneContext sceneCtx;
extern glm::mat4 view;
extern glm::mat4 projection;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureMouse) return;

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // --- LÓGICA DE PICKING (Botão Esquerdo) ---
    // Se Ctrl NÃO está pressionado, faz picking normalmente
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !(mods & GLFW_MOD_CONTROL)) {
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
        if (animCtrl.getCurrentMode() == AnimationController::Mode3D) {
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        }

        int closestIdx = -1;
        float closestDist = std::numeric_limits<float>::max();

        for (size_t i = 0; i < tree.segments.size(); ++i) {
            const auto& seg = tree.segments[i];
            glm::vec3 a = tree.nodes[seg.indexA].position;
            glm::vec3 b = tree.nodes[seg.indexB].position;
            // Clipping check
            if (animCtrl.clipping.enabled) {
                glm::vec3 tempA = a, tempB = b;
                if (!ClippingUtils::clipSegment(tempA, tempB, animCtrl.clipping.min, animCtrl.clipping.max)) {
                    continue;
                }
            }
            glm::vec4 localA = glm::vec4(a, 1.0f);
            glm::vec4 localB = glm::vec4(b, 1.0f);
            glm::vec3 worldA = glm::vec3(model * localA);
            glm::vec3 worldB = glm::vec3(model * localB);
            float hitRadius = std::max(seg.radius * animCtrl.radiusScale * 1.1f, 0.0001f);
            float outDist;
            if (PickingUtils::rayIntersectsSegment(rayOrigin, rayDir, worldA, worldB, hitRadius, outDist)) {
                if (outDist < closestDist) {
                    closestDist = outDist;
                    closestIdx = static_cast<int>(i);
                }
            }
        }

        animCtrl.selectSegment(closestIdx, tree);
    }

    // Se Ctrl está pressionado E botão esquerdo, trata como pan (mão)
    // (aciona o pan do botão direito, mas com o esquerdo+Ctrl)
    if (button == GLFW_MOUSE_BUTTON_LEFT && (mods & GLFW_MOD_CONTROL)) {
        // Simula botão direito para Camera
        camera.processMouseButton(GLFW_MOUSE_BUTTON_RIGHT, action, xpos, ypos);
        return;
    }

    camera.processMouseButton(button, action, xpos, ypos);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    camera.processMouseMovement(xpos, ypos);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureMouse) return;
    camera.processMouseScroll(static_cast<float>(yoffset));
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// Global for picking callback access
AnimationController animCtrl;
ArterialTree tree;
SceneContext sceneCtx;
glm::mat4 view;
glm::mat4 projection;

int main() {
    // 1. Inicialização do GLFW
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Arterial Tree Visualization", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // 2. Inicialização do GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    // Cursor feedback for panning
    GLFWcursor* handCursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    GLFWcursor* arrowCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);

    // 3. Inicialização do ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.FontGlobalScale = 1.5f;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Configuração OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 4. Objetos do Domínio

    Shader shader("../shaders/vertex.glsl", "../shaders/fragment.glsl");
    Shader lineShader("../shaders/line_vertex.glsl", "../shaders/line_fragment.glsl");
    TreeRenderer renderer;
    MenuController menuCtrl;
    sceneCtx.init();
    view = glm::mat4(1.0f);
    projection = glm::mat4(1.0f);

    double lastTime = glfwGetTime();

    // 6. Loop Principal

    while (!glfwWindowShouldClose(window)) {
        // --- SCREENSHOT LOGIC ---
        bool isSnapshot = animCtrl.isScreenshotRequested();

        // 1. Clear color (transparent for screenshot, normal otherwise)
        if (isSnapshot)
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        else
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 2. Animation logic
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;
        processInput(window);
        animCtrl.update(deltaTime, tree, renderer);

        // 3. Update aspect ratio and view/projection
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        float aspect = (height > 0) ? (float)width / (float)height : 1.0f;
        view = camera.getViewMatrix();
        if (animCtrl.useOrthographic) {
            float orthoHeight = glm::length(camera.getPosition());
            orthoHeight = std::max(orthoHeight, 0.1f);
            float orthoWidth = orthoHeight * aspect;
            projection = glm::ortho(-orthoWidth, orthoWidth, -orthoHeight, orthoHeight, 0.1f, 100.0f);
        } else {
            projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
        }

        // 4. Cursor feedback for panning
        bool isPanning = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
        glfwSetCursor(window, isPanning ? handCursor : arrowCursor);

        // 5. Visual update if needed
        if (animCtrl.isVisualDirty()) {
            if (animCtrl.getCurrentMode() == AnimationController::ModeWireframe) {
                renderer.initWireframe(tree.nodes, tree.segments,
                    animCtrl.clipping.enabled,
                    animCtrl.clipping.min,
                    animCtrl.clipping.max);
            } else {
                renderer.init(tree,
                    animCtrl.radiusScale,
                    animCtrl.showSpheres,
                    animCtrl.clipping.enabled,
                    animCtrl.clipping.min,
                    animCtrl.clipping.max);
            }
            animCtrl.resetVisualDirty();
        }

        // 6. Set shader uniforms and draw scene
        glm::mat4 model = glm::mat4(1.0f);
        if (animCtrl.getCurrentMode() == AnimationController::Mode3D) {
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

        if (animCtrl.getCurrentMode() == AnimationController::ModeWireframe) {
            lineShader.use();
            lineShader.setMat4("model", model);
            lineShader.setMat4("view", view);
            lineShader.setMat4("projection", projection);
            lineShader.setInt("selectedSegmentID", animCtrl.getSelectedSegment());
            lineShader.setFloat("alpha", animCtrl.transparency);
            renderer.drawWireframe(lineShader, view, projection, model, animCtrl.lineWidth, animCtrl.getSelectedSegment());
        } else {
            renderer.draw(shader, view, projection, model, animCtrl.getSelectedSegment());
        }

        // 7. Draw grid and gizmo
        if (animCtrl.showGrid) {
            sceneCtx.drawGrid(view, projection);
        }
        if (animCtrl.showGizmo) {
            sceneCtx.drawGizmo(view, projection, width, height);
        }

        // 8. Render UI (hide main menu if snapshot)
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        menuCtrl.render(animCtrl, tree, renderer, isSnapshot);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // 9. Screenshot capture (before swap)
        if (isSnapshot) {
            ScreenshotUtils::saveScreenshot(width, height);
            animCtrl.resetScreenshotRequest();
        }

        // 10. Swap and poll
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glfwDestroyCursor(handCursor);
    glfwDestroyCursor(arrowCursor);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}