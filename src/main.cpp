#include "MenuController.hpp"
#include "AnimationController.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "VtkReader.hpp"
#include "shader.hpp"
#include "TreeRenderer.hpp"
#include "SceneContext.hpp"

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
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        
        // 1. Configuração DPI
        int winWidth, winHeight;
        glfwGetWindowSize(window, &winWidth, &winHeight);
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

        float xScale = (float)fbWidth / (float)winWidth;
        float yScale = (float)fbHeight / (float)winHeight;
        double mouseX_FB = xpos * xScale;
        double mouseY_FB = ypos * yScale;

        // 2. IMPORTANTE: Pegar a View Matrix ATUALIZADA da câmera agora
        // Não use a variável global 'view', pois ela pode estar atrasada 1 frame.
        glm::mat4 currentView = camera.getViewMatrix();

        // 3. Gera a direção do Raio
        glm::vec3 rayDir = PickingUtils::getRayFromMouse(mouseX_FB, mouseY_FB, fbWidth, fbHeight, currentView, projection);
        
        // 4. CORREÇÃO CRÍTICA: Origem do Raio
        // A função camera.getPosition() ignora o Pan.
        // A posição real da câmera no mundo é a transladação da View Matrix Inversa.
        glm::mat4 invView = glm::inverse(currentView);
        glm::vec3 rayOrigin = glm::vec3(invView[3]); // A 4ª coluna contém a posição X,Y,Z da câmera no mundo

        // 5. Matriz Model (-90 graus em 3D)
        glm::mat4 model = glm::mat4(1.0f);
        if (animCtrl.getCurrentMode() == AnimationController::Mode3D) {
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        }

        int closestIdx = -1;
        float closestDist = std::numeric_limits<float>::max();

        for (size_t i = 0; i < tree.segments.size(); ++i) {
            const auto& seg = tree.segments[i];
            
            glm::vec4 localA = glm::vec4(tree.nodes[seg.indexA].position, 1.0f);
            glm::vec4 localB = glm::vec4(tree.nodes[seg.indexB].position, 1.0f);
            glm::vec3 worldA = glm::vec3(model * localA);
            glm::vec3 worldB = glm::vec3(model * localB);

            // Hitbox
            float hitRadius = std::max(seg.radius * animCtrl.radiusScale * 2.0f, 0.001f); 

            float outDist;
            if (PickingUtils::rayIntersectsSegment(rayOrigin, rayDir, worldA, worldB, hitRadius, outDist)) {
                if (outDist < closestDist) {
                    closestDist = outDist;
                    closestIdx = static_cast<int>(i);
                }
            }
        }
        
        animCtrl.selectSegment(closestIdx);
    }

    // --- LÓGICA DE CÂMERA (Repasse) ---
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

    GLFWwindow* window = glfwCreateWindow(800, 600, "Arterial Tree Visualization", nullptr, nullptr);
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
    io.FontGlobalScale = 1.5f;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Configuração OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 4. Objetos do Domínio

    Shader shader("../shaders/vertex.glsl", "../shaders/fragment.glsl");
    TreeRenderer renderer;
    MenuController menuCtrl;
    sceneCtx.init();
    view = glm::mat4(1.0f);
    projection = glm::mat4(1.0f);

    double lastTime = glfwGetTime();

    // 6. Loop Principal
    while (!glfwWindowShouldClose(window)) {
        // Recalculate model matrix every frame for mode switching
        glm::mat4 model = glm::mat4(1.0f);
        if (animCtrl.getCurrentMode() == AnimationController::Mode3D) {
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        }
        // Auto-reset camera if requested by AnimationController
        if (animCtrl.shouldResetCamera()) {
            camera.reset();
            animCtrl.ackCameraReset();
        }
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        processInput(window);

        // Limpeza
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Lógica de Animação
        animCtrl.update(deltaTime, tree, renderer);


        // Atualiza aspect ratio e view/projection
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height); // Fix initial clipping on some systems
        float aspect = (height > 0) ? (float)width / (float)height : 1.0f;
        projection = glm::perspective(glm::radians(45.0f), aspect, 0.001f, 100.0f);
        view = camera.getViewMatrix();

        // Cursor feedback for panning
        bool isPanning = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
        glfwSetCursor(window, isPanning ? handCursor : arrowCursor);

        // Atualiza visualização se necessário
        if (animCtrl.isVisualDirty()) {
            renderer.init(tree, animCtrl.radiusScale, animCtrl.showSpheres);
            animCtrl.resetVisualDirty();
        }

        // Set shader uniforms
        shader.use();
        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setVec3("lightPos", glm::vec3(animCtrl.lightPos[0], animCtrl.lightPos[1], animCtrl.lightPos[2]));
        shader.setVec3("viewPos", camera.getPosition());
        shader.setFloat("alpha", animCtrl.transparency);
        shader.setInt("lightingMode", animCtrl.lightingMode);

        renderer.draw(shader, view, projection, model);

        // Draw grid and gizmo
        if (animCtrl.showGrid) {
            sceneCtx.drawGrid(view, projection);
        }
        if (animCtrl.showGizmo) {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            sceneCtx.drawGizmo(view, projection, width, height);
        }

        // Renderização da Interface
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        menuCtrl.render(animCtrl, tree, renderer);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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