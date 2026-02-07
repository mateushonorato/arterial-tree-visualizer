#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h> // MUST be included before GLFW
#include <GLFW/glfw3.h>
#include <iostream>
#include "VtkReader.hpp"
#include "shader.hpp"
#include "TreeRenderer.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Callback: Adjust viewport on window resize
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Process input: Close window on ESC
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main() {
    // Test VtkReader
    ArterialTree tree;
    if (VtkReader::load("../data/TP1_2D/Nterm_064/tree2D_Nterm0064_step0008.vtk", tree)) {
        std::cout << "Loaded Tree! Nodes: " << tree.nodes.size() << ", Segments: " << tree.segments.size() << std::endl;
    } else {
        std::cerr << "Failed to load VTK file." << std::endl;
    }

    // Matrices (declare here, initialize after GLAD)
    glm::mat4 view, projection, model;
    Shader* shader = nullptr;
    TreeRenderer* renderer = nullptr;

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    // Configure GLFW: OpenGL 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Arterial Tree Visualization", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // ImGui initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    std::cout << "OpenGL Initialized" << std::endl;
    std::cout << "GL_VENDOR:   " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "GL_RENDERER: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "GL_VERSION:  " << glGetString(GL_VERSION) << std::endl;


    // Setup matrices and OpenGL objects after GLAD
    view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.2f));
    projection = glm::perspective(glm::radians(45.0f), 800.0f/600.0f, 0.01f, 10.0f);
    model = glm::mat4(1.0f);
    shader = new Shader("../shaders/vertex.glsl", "../shaders/fragment.glsl");
    renderer = new TreeRenderer();
    renderer->init(tree);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        renderer->draw(*shader, view, projection, model);

        // ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Controle de Animação");
        ImGui::Text("Sistema Pronto!");
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete shader;
    delete renderer;

    // ImGui cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
