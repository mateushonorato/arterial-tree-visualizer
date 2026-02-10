// Exercise 18 - Cyrus-Beck line clipping against a convex polygon (hexagon)
// Left-click to set P0, right-click to set P1, press 'C' to clear.

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>

static int SCR_W = 800;
static int SCR_H = 600;

static glm::vec2 P0, P1;
static bool hasP0 = false, hasP1 = false;

// Simple shader (uniform color)
static const char* vs_src = R"glsl(
#version 330 core
layout(location = 0) in vec2 aPos;
uniform mat4 proj;
void main(){ gl_Position = proj * vec4(aPos, 0.0, 1.0); }
)glsl";

static const char* fs_src = R"glsl(
#version 330 core
uniform vec3 uColor;
out vec4 FragColor;
void main(){ FragColor = vec4(uColor,1.0); }
)glsl";

static void cursor_to_ndc(GLFWwindow* w, double x, double y, glm::vec2 &out){
    out.x = (float)((2.0 * x) / SCR_W - 1.0);
    out.y = (float)(1.0 - (2.0 * y) / SCR_H);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
    if(action != GLFW_PRESS) return;
    double x,y; glfwGetCursorPos(window,&x,&y);
    glm::vec2 p; cursor_to_ndc(window,x,y,p);
    if(button == GLFW_MOUSE_BUTTON_LEFT){ P0 = p; hasP0 = true; }
    if(button == GLFW_MOUSE_BUTTON_RIGHT){ P1 = p; hasP1 = true; }
}

static void key_callback(GLFWwindow* window, int key, int sc, int action, int mods){
    if(action != GLFW_PRESS) return;
    if(key == GLFW_KEY_C){ hasP0 = hasP1 = false; }
    if(key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// Cyrus-Beck clipping for convex polygon (vertices in CCW order)
// Returns pair<bool, pair<glm::vec2, glm::vec2>>: accepted and clipped segment
static std::pair<bool, std::pair<glm::vec2,glm::vec2>> cyrus_beck_clip(const glm::vec2 &p0, const glm::vec2 &p1, const std::vector<glm::vec2> &poly){
    float tE = 0.0f; // entering
    float tL = 1.0f; // leaving
    glm::vec2 d = p1 - p0;

    for(size_t i=0;i<poly.size();++i){
        glm::vec2 vi = poly[i];
        glm::vec2 vj = poly[(i+1)%poly.size()];
        glm::vec2 edge = vj - vi;
        // inward normal for CCW polygon
        glm::vec2 n = glm::normalize(glm::vec2(edge.y, -edge.x));

        float numerator = glm::dot(n, vi - p0);
        float denominator = glm::dot(n, d);

        if (fabs(denominator) < 1e-6f){
            if (numerator < 0.0f) {
                return {false, {{0,0},{0,0}}}; // parallel and outside
            } else {
                continue; // parallel and inside, no restriction
            }
        }

        float t = numerator / denominator;
        if(denominator < 0.0f){
            // potential entering
            tE = glm::max(tE, t);
        } else {
            // potential leaving
            tL = glm::min(tL, t);
        }
        if(tE > tL) return {false, {{0,0},{0,0}}};
    }

    glm::vec2 Cp0 = p0 + d * tE;
    glm::vec2 Cp1 = p0 + d * tL;
    return {true, {Cp0, Cp1}};
}

int main(){
    if(!glfwInit()){ std::cerr<<"glfwInit failed\n"; return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_W, SCR_H, "ex18_cyrus_beck", nullptr, nullptr);
    if(!window){ std::cerr<<"window failed\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){ std::cerr<<"glad failed\n"; return -1; }

    // create simple program
    GLuint vs = glCreateShader(GL_VERTEX_SHADER); glShaderSource(vs,1,&vs_src,nullptr); glCompileShader(vs); GLint ok; glGetShaderiv(vs,GL_COMPILE_STATUS,&ok); if(!ok){ char b[1024]; glGetShaderInfoLog(vs,1024,nullptr,b); std::cerr<<"vs: "<<b; }
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER); glShaderSource(fs,1,&fs_src,nullptr); glCompileShader(fs); glGetShaderiv(fs,GL_COMPILE_STATUS,&ok); if(!ok){ char b[1024]; glGetShaderInfoLog(fs,1024,nullptr,b); std::cerr<<"fs: "<<b; }
    GLuint prog = glCreateProgram(); glAttachShader(prog,vs); glAttachShader(prog,fs); glLinkProgram(prog); glGetProgramiv(prog,GL_LINK_STATUS,&ok); if(!ok){ char b[1024]; glGetProgramInfoLog(prog,1024,nullptr,b); std::cerr<<"prog: "<<b; }
    glDeleteShader(vs); glDeleteShader(fs);

    // convex polygon: regular hexagon CCW
    std::vector<glm::vec2> hex;
    float R = 0.6f;
    for(int i=0;i<6;++i){ float ang = glm::radians(60.0f * i + 30.0f); hex.emplace_back(R * cos(ang), R * sin(ang)); }

    // polygon VAO
    GLuint polyVAO, polyVBO; glGenVertexArrays(1,&polyVAO); glGenBuffers(1,&polyVBO);
    glBindVertexArray(polyVAO);
    glBindBuffer(GL_ARRAY_BUFFER, polyVBO);
    glBufferData(GL_ARRAY_BUFFER, hex.size()*sizeof(glm::vec2), hex.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,(void*)0); glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // line VAO for dynamic segments
    GLuint lineVAO, lineVBO; glGenVertexArrays(1,&lineVAO); glGenBuffers(1,&lineVBO);
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec2), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,(void*)0); glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    glUseProgram(prog);
    GLint projLoc = glGetUniformLocation(prog, "proj");
    GLint colorLoc = glGetUniformLocation(prog, "uColor");
    glm::mat4 proj = glm::ortho(-1.0f,1.0f,-1.0f,1.0f);
    glUniformMatrix4fv(projLoc,1,GL_FALSE, glm::value_ptr(proj));

    while(!glfwWindowShouldClose(window)){
        glfwGetFramebufferSize(window,&SCR_W,&SCR_H);
        glViewport(0,0,SCR_W,SCR_H);

        glClearColor(0.12f,0.12f,0.12f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw polygon outline in white
        glBindVertexArray(polyVAO);
        glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
        glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)hex.size());

        // draw original segment if any
        if(hasP0 && hasP1){
            glm::vec2 seg[2] = {P0, P1};
            glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(seg), seg);
            glBindVertexArray(lineVAO);
            glLineWidth(1.0f);
            glUniform3f(colorLoc, 0.8f, 0.8f, 0.8f);
            glDrawArrays(GL_LINES, 0, 2);

            // compute clipping
            auto res = cyrus_beck_clip(P0,P1,hex);
            if(res.first){
                glm::vec2 clipped[2] = { res.second.first, res.second.second };
                glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(clipped), clipped);
                glBindVertexArray(lineVAO);
                glLineWidth(3.0f);
                glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f);
                glDrawArrays(GL_LINES, 0, 2);
                glLineWidth(1.0f);
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1,&polyVAO); glDeleteBuffers(1,&polyVBO);
    glDeleteVertexArrays(1,&lineVAO); glDeleteBuffers(1,&lineVBO);
    glDeleteProgram(prog);
    glfwDestroyWindow(window); glfwTerminate();
    return 0;
}
