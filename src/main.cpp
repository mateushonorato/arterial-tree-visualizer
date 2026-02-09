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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Camera.hpp"


// Global camera instance
Camera camera;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureMouse) return;
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
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
    ArterialTree tree;
    Shader shader("../shaders/vertex.glsl", "../shaders/fragment.glsl");
    TreeRenderer renderer;
    
    // 5. Controladores MVC
    AnimationController animCtrl;
    MenuController menuCtrl;

    // Matrizes
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

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
            renderer.init(tree, animCtrl.radiusScale);
            animCtrl.resetVisualDirty();
        }

        // Set shader uniforms
        shader.use();
        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setVec3("lightPos", glm::vec3(animCtrl.lightPos[0], animCtrl.lightPos[1], animCtrl.lightPos[2]));
        shader.setVec3("viewPos", camera.getPosition());
        shader.setVec3("objectColor", glm::vec3(animCtrl.objectColor[0], animCtrl.objectColor[1], animCtrl.objectColor[2]));
        shader.setFloat("alpha", animCtrl.transparency);

        renderer.draw(shader, view, projection, model);

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