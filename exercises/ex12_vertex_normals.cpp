// Exercise 12 - Vertex normals (smooth shading / Gouraud in vertex shader)
// Builds an indexed cube mesh, computes per-vertex normals by averaging face normals,
// and renders using Gouraud shading (lighting computed in vertex shader).

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>

static const char* vertexSrc = R"glsl(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vColor; // computed lighting color (Gouraud)

void main(){
    // transform position
    vec4 worldPos = model * vec4(aPos, 1.0);
    gl_Position = projection * view * worldPos;

    // transform normal to world space
    mat3 normalMat = transpose(inverse(mat3(model)));
    vec3 N = normalize(normalMat * aNormal);

    // simple directional light (in world space)
    vec3 lightPos = vec3(2.0, 2.0, 2.0);
    vec3 L = normalize(lightPos - worldPos.xyz);
    float diff = max(dot(N, L), 0.0);

    vec3 baseColor = vec3(0.8, 0.6, 0.4);
    vec3 ambient = 0.1 * baseColor;
    vec3 diffuse = diff * baseColor;

    vColor = ambient + diffuse;
}
)glsl";

static const char* fragmentSrc = R"glsl(
#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main(){ FragColor = vec4(vColor, 1.0); }
)glsl";

static void checkCompile(GLuint sh, const char* name){
    GLint ok; glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if(!ok){ char buf[1024]; glGetShaderInfoLog(sh, 1024, nullptr, buf); std::cerr<<name<<" compile: "<<buf<<"\n"; }
}
static void checkLink(GLuint p){ GLint ok; glGetProgramiv(p, GL_LINK_STATUS, &ok); if(!ok){ char buf[1024]; glGetProgramInfoLog(p,1024,nullptr,buf); std::cerr<<"link: "<<buf<<"\n";} }

int main(){
    if(!glfwInit()){ std::cerr<<"glfwInit failed\n"; return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* win = glfwCreateWindow(800,600,"ex12_vertex_normals",nullptr,nullptr);
    if(!win){ std::cerr<<"window failed\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(win);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){ std::cerr<<"glad failed\n"; return -1; }

    glEnable(GL_DEPTH_TEST);

    // Indexed cube: 8 positions
    std::vector<glm::vec3> positions = {
        {-0.5f, -0.5f, -0.5f}, // 0
        { 0.5f, -0.5f, -0.5f}, // 1
        { 0.5f,  0.5f, -0.5f}, // 2
        {-0.5f,  0.5f, -0.5f}, // 3
        {-0.5f, -0.5f,  0.5f}, // 4
        { 0.5f, -0.5f,  0.5f}, // 5
        { 0.5f,  0.5f,  0.5f}, // 6
        {-0.5f,  0.5f,  0.5f}  // 7
    };

    // 12 triangles (36 indices)
    std::vector<unsigned int> idx = {
        // front (+Z)
        4,5,6,  4,6,7,
        // back (-Z)
        1,0,3,  1,3,2,
        // left (-X)
        0,4,7,  0,7,3,
        // right (+X)
        5,1,2,  5,2,6,
        // top (+Y)
        3,7,6,  3,6,2,
        // bottom (-Y)
        0,1,5,  0,5,4
    };

    // compute normals per vertex
    std::vector<glm::vec3> normals(positions.size(), glm::vec3(0.0f));
    for(size_t t=0;t<idx.size();t+=3){
        glm::vec3 p0 = positions[idx[t+0]];
        glm::vec3 p1 = positions[idx[t+1]];
        glm::vec3 p2 = positions[idx[t+2]];
        glm::vec3 e1 = p1 - p0;
        glm::vec3 e2 = p2 - p0;
        glm::vec3 faceN = glm::normalize(glm::cross(e1,e2));
        normals[idx[t+0]] += faceN;
        normals[idx[t+1]] += faceN;
        normals[idx[t+2]] += faceN;
    }
    for(auto &n : normals) n = glm::normalize(n);

    // build interleaved vertex buffer (pos + normal)
    std::vector<float> vboData;
    for(size_t i=0;i<positions.size();++i){
        vboData.push_back(positions[i].x);
        vboData.push_back(positions[i].y);
        vboData.push_back(positions[i].z);
        vboData.push_back(normals[i].x);
        vboData.push_back(normals[i].y);
        vboData.push_back(normals[i].z);
    }

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);
    glGenBuffers(1,&EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vboData.size()*sizeof(float), vboData.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size()*sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(3*sizeof(float))); glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // compile shaders
    GLuint vs = glCreateShader(GL_VERTEX_SHADER); glShaderSource(vs,1,&vertexSrc,nullptr); glCompileShader(vs); checkCompile(vs,"vs");
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER); glShaderSource(fs,1,&fragmentSrc,nullptr); glCompileShader(fs); checkCompile(fs,"fs");
    GLuint prog = glCreateProgram(); glAttachShader(prog,vs); glAttachShader(prog,fs); glLinkProgram(prog); checkLink(prog);
    glDeleteShader(vs); glDeleteShader(fs);

    glUseProgram(prog);

    GLint modelLoc = glGetUniformLocation(prog,"model");
    GLint viewLoc = glGetUniformLocation(prog,"view");
    GLint projLoc = glGetUniformLocation(prog,"projection");

    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,0.0f,-3.0f));
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f/600.0f, 0.1f, 100.0f);
    glUniformMatrix4fv(viewLoc,1,GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc,1,GL_FALSE, glm::value_ptr(proj));

    while(!glfwWindowShouldClose(win)){
        if(glfwGetKey(win, GLFW_KEY_ESCAPE)==GLFW_PRESS) glfwSetWindowShouldClose(win, GLFW_TRUE);

        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        float t = (float)glfwGetTime();
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), t * glm::radians(30.0f), glm::vec3(1.0f,1.0f,0.0f));
        glUniformMatrix4fv(modelLoc,1,GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)idx.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glDeleteProgram(prog);
    glDeleteBuffers(1,&VBO); glDeleteBuffers(1,&EBO); glDeleteVertexArrays(1,&VAO);
    glfwDestroyWindow(win); glfwTerminate();
    return 0;
}
