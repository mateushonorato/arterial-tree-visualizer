// Exercise 10 - Flashlight (spotlight in view space)
// Spotlight parameters:
// color: yellow (1,1,0)
// cutoff: cos(radians(30.0))
// exponent: 15.0
// quadratic attenuation kq = 0.01

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

static const char* vertexShaderSrc = R"glsl(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;

out vec3 vPosView;
out vec3 vNormalView;
out vec3 vColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    mat4 mv = view * model;
    vec4 posView4 = mv * vec4(aPos, 1.0);
    vPosView = posView4.xyz;
    vNormalView = mat3(transpose(inverse(mv))) * aNormal;
    vColor = aColor;
    gl_Position = projection * posView4;
}
)glsl";

static const char* fragmentShaderSrc = R"glsl(
#version 330 core
in vec3 vPosView;
in vec3 vNormalView;
in vec3 vColor;
out vec4 FragColor;

// Spotlight parameters (in view space)
const vec3 lightColor = vec3(1.0, 1.0, 0.0);
const float cutoff = cos(radians(30.0));
const float exponent = 15.0;
const float kq = 0.01;

void main() {
    vec3 N = normalize(vNormalView);
    vec3 lightPos = vec3(0.0, 0.0, 0.0); // camera origin in view space
    vec3 lightToFrag = normalize(vPosView - lightPos); // vector from light to fragment
    vec3 L = normalize(lightPos - vPosView); // vector from fragment to light

    // Spotlight direction (in view space): camera looks towards -Z
    vec3 spotDir = normalize(vec3(0.0, 0.0, -1.0));

    float theta = dot(lightToFrag, spotDir); // cos angle between light->frag and spotDir
    float spotEffect = 0.0;
    if (theta > cutoff) {
        spotEffect = pow(theta, exponent);
    }

    float distance = length(lightPos - vPosView);
    float attenuation = 1.0 / (1.0 + kq * distance * distance);

    float diff = max(dot(N, L), 0.0);

    vec3 ambient = 0.05 * vColor;
    vec3 diffuse = diff * vColor * lightColor * spotEffect * attenuation;

    vec3 result = ambient + diffuse;
    FragColor = vec4(result, 1.0);
}
)glsl";

static void checkCompile(GLuint shader, const char* name) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, info);
        std::cerr << name << " compile error:\n" << info << std::endl;
    }
}
static void checkLink(GLuint prog) {
    GLint success;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        char info[1024];
        glGetProgramInfoLog(prog, 1024, nullptr, info);
        std::cerr << "Program link error:\n" << info << std::endl;
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(800, 600, "Exercise 10 - Flashlight", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // Build program
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSrc, nullptr);
    glCompileShader(vs);
    checkCompile(vs, "Vertex Shader");

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSrc, nullptr);
    glCompileShader(fs);
    checkCompile(fs, "Fragment Shader");

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    checkLink(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    // Cube data: position (3) + normal (3) + color (3) => 9 floats per vertex, 36 vertices
    float vertices[] = {
        // +X face (right) normal (1,0,0) color magenta
         0.5f, -0.5f,  0.5f,   1.0f,0.0f,0.0f,    1.0f,0.0f,1.0f,
         0.5f,  0.5f,  0.5f,   1.0f,0.0f,0.0f,    1.0f,0.0f,1.0f,
         0.5f, -0.5f, -0.5f,   1.0f,0.0f,0.0f,    1.0f,0.0f,1.0f,

         0.5f,  0.5f,  0.5f,   1.0f,0.0f,0.0f,    1.0f,0.0f,1.0f,
         0.5f,  0.5f, -0.5f,   1.0f,0.0f,0.0f,    1.0f,0.0f,1.0f,
         0.5f, -0.5f, -0.5f,   1.0f,0.0f,0.0f,    1.0f,0.0f,1.0f,

        // -X face (left) normal (-1,0,0) color cyan
        -0.5f, -0.5f, -0.5f,  -1.0f,0.0f,0.0f,    0.0f,1.0f,1.0f,
        -0.5f,  0.5f, -0.5f,  -1.0f,0.0f,0.0f,    0.0f,1.0f,1.0f,
        -0.5f, -0.5f,  0.5f,  -1.0f,0.0f,0.0f,    0.0f,1.0f,1.0f,

        -0.5f,  0.5f, -0.5f,  -1.0f,0.0f,0.0f,    0.0f,1.0f,1.0f,
        -0.5f,  0.5f,  0.5f,  -1.0f,0.0f,0.0f,    0.0f,1.0f,1.0f,
        -0.5f, -0.5f,  0.5f,  -1.0f,0.0f,0.0f,    0.0f,1.0f,1.0f,

        // +Y face (top) normal (0,1,0) color red
        -0.5f,  0.5f, -0.5f,   0.0f,1.0f,0.0f,    1.0f,0.0f,0.0f,
         0.5f,  0.5f, -0.5f,   0.0f,1.0f,0.0f,    1.0f,0.0f,0.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,1.0f,0.0f,    1.0f,0.0f,0.0f,

         0.5f,  0.5f, -0.5f,   0.0f,1.0f,0.0f,    1.0f,0.0f,0.0f,
         0.5f,  0.5f,  0.5f,   0.0f,1.0f,0.0f,    1.0f,0.0f,0.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,1.0f,0.0f,    1.0f,0.0f,0.0f,

        // -Y face (bottom) normal (0,-1,0) color green
        -0.5f, -0.5f,  0.5f,   0.0f,-1.0f,0.0f,   0.0f,1.0f,0.0f,
         0.5f, -0.5f,  0.5f,   0.0f,-1.0f,0.0f,   0.0f,1.0f,0.0f,
        -0.5f, -0.5f, -0.5f,   0.0f,-1.0f,0.0f,   0.0f,1.0f,0.0f,

         0.5f, -0.5f,  0.5f,   0.0f,-1.0f,0.0f,   0.0f,1.0f,0.0f,
         0.5f, -0.5f, -0.5f,   0.0f,-1.0f,0.0f,   0.0f,1.0f,0.0f,
        -0.5f, -0.5f, -0.5f,   0.0f,-1.0f,0.0f,   0.0f,1.0f,0.0f,

        // +Z face (front) normal (0,0,1) color blue
        -0.5f, -0.5f,  0.5f,   0.0f,0.0f,1.0f,    0.0f,0.0f,1.0f,
         0.5f, -0.5f,  0.5f,   0.0f,0.0f,1.0f,    0.0f,0.0f,1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,0.0f,1.0f,    0.0f,0.0f,1.0f,

         0.5f, -0.5f,  0.5f,   0.0f,0.0f,1.0f,    0.0f,0.0f,1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,0.0f,1.0f,    0.0f,0.0f,1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,0.0f,1.0f,    0.0f,0.0f,1.0f,

        // -Z face (back) normal (0,0,-1) color yellow
         0.5f, -0.5f, -0.5f,   0.0f,0.0f,-1.0f,   1.0f,1.0f,0.0f,
        -0.5f, -0.5f, -0.5f,   0.0f,0.0f,-1.0f,   1.0f,1.0f,0.0f,
         0.5f,  0.5f, -0.5f,   0.0f,0.0f,-1.0f,   1.0f,1.0f,0.0f,

        -0.5f, -0.5f, -0.5f,   0.0f,0.0f,-1.0f,   1.0f,1.0f,0.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,0.0f,-1.0f,   1.0f,1.0f,0.0f,
         0.5f,  0.5f, -0.5f,   0.0f,0.0f,-1.0f,   1.0f,1.0f,0.0f
    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLsizei stride = 9 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glUseProgram(program);

    GLint modelLoc = glGetUniformLocation(program, "model");
    GLint viewLoc = glGetUniformLocation(program, "view");
    GLint projLoc = glGetUniformLocation(program, "projection");

    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    float aspect = width / (float)height;
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float time = (float)glfwGetTime();
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), time * glm::radians(25.0f), glm::vec3(1.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(program);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
