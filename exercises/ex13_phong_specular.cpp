// Exercise 13 - Gouraud vs Phong specular demonstration
// Press 'S' to toggle shading mode (Gouraud <-> Phong)

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>

enum class ShadingMode { Gouraud, Phong };
static ShadingMode shading = ShadingMode::Gouraud;

void key_cb(GLFWwindow* w, int key, int sc, int action, int mods){
    if(action==GLFW_PRESS){
        if(key==GLFW_KEY_S){
            shading = (shading==ShadingMode::Gouraud) ? ShadingMode::Phong : ShadingMode::Gouraud;
            std::cout << "Shading: " << (shading==ShadingMode::Gouraud?"Gouraud":"Phong") << '\n';
        } else if(key==GLFW_KEY_ESCAPE){
            glfwSetWindowShouldClose(w, GLFW_TRUE);
        }
    }
}

static const char* gouraud_vs = R"glsl(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightPos; // world space
uniform vec3 viewPos;  // world space
uniform float shininess;
uniform float kq;

out vec3 vColor;

void main(){
    vec4 worldPos4 = model * vec4(aPos,1.0);
    vec3 worldPos = worldPos4.xyz;
    mat3 normalMat = transpose(inverse(mat3(model)));
    vec3 N = normalize(normalMat * aNormal);

    // lighting (Blinn-Phong)
    vec3 lightDir = normalize(lightPos - worldPos);
    float diff = max(dot(N, lightDir), 0.0);

    vec3 V = normalize(viewPos - worldPos);
    vec3 H = normalize(lightDir + V);
    float spec = pow(max(dot(N,H), 0.0), shininess);

    float dist = length(lightPos - worldPos);
    float attenuation = 1.0 / (1.0 + kq * dist * dist);

    vec3 base = vec3(0.8,0.6,0.4);
    vec3 ambient = 0.1 * base;
    vec3 diffuse = diff * base;
    vec3 specular = spec * vec3(1.0);

    vec3 color = ambient + attenuation * (diffuse + specular);
    vColor = color;
    gl_Position = projection * view * worldPos4;
}
)glsl";

static const char* gouraud_fs = R"glsl(
#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main(){ FragColor = vec4(vColor,1.0); }
)glsl";

static const char* phong_vs = R"glsl(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;

void main(){
    vec4 worldPos4 = model * vec4(aPos,1.0);
    FragPos = worldPos4.xyz;
    Normal = mat3(transpose(inverse(mat3(model)))) * aNormal;
    gl_Position = projection * view * worldPos4;
}
)glsl";

static const char* phong_fs = R"glsl(
#version 330 core
in vec3 FragPos;
in vec3 Normal;
out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float shininess;
uniform float kq;

void main(){
    vec3 N = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(N, lightDir), 0.0);

    vec3 V = normalize(viewPos - FragPos);
    vec3 H = normalize(lightDir + V);
    float spec = pow(max(dot(N,H), 0.0), shininess);

    float dist = length(lightPos - FragPos);
    float attenuation = 1.0 / (1.0 + kq * dist * dist);

    vec3 base = vec3(0.8,0.6,0.4);
    vec3 ambient = 0.1 * base;
    vec3 diffuse = diff * base;
    vec3 specular = spec * vec3(1.0);

    vec3 color = ambient + attenuation * (diffuse + specular);
    FragColor = vec4(color,1.0);
}
)glsl";

static void checkCompile(GLuint s,const char* name){ GLint ok; glGetShaderiv(s,GL_COMPILE_STATUS,&ok); if(!ok){ char b[1024]; glGetShaderInfoLog(s,1024,nullptr,b); std::cerr<<name<<" shader: "<<b<<"\n"; }}
static void checkLink(GLuint p){ GLint ok; glGetProgramiv(p,GL_LINK_STATUS,&ok); if(!ok){ char b[1024]; glGetProgramInfoLog(p,1024,nullptr,b); std::cerr<<"program link: "<<b<<"\n"; }}

int main(){
    if(!glfwInit()){ std::cerr<<"glfwInit failed\n"; return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* win = glfwCreateWindow(800,600,"ex13_phong_specular",nullptr,nullptr);
    if(!win){ std::cerr<<"window fail\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(win);
    glfwSetKeyCallback(win, key_cb);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){ std::cerr<<"glad fail\n"; return -1; }

    glEnable(GL_DEPTH_TEST);

    // Indexed cube same as ex12
    std::vector<glm::vec3> pos = {
        {-0.5f,-0.5f,-0.5f}, {0.5f,-0.5f,-0.5f}, {0.5f,0.5f,-0.5f}, {-0.5f,0.5f,-0.5f},
        {-0.5f,-0.5f,0.5f},  {0.5f,-0.5f,0.5f},  {0.5f,0.5f,0.5f},  {-0.5f,0.5f,0.5f}
    };
    std::vector<unsigned int> idx = { 4,5,6,4,6,7, 1,0,3,1,3,2, 0,4,7,0,7,3, 5,1,2,5,2,6, 3,7,6,3,6,2, 0,1,5,0,5,4 };

    std::vector<glm::vec3> normals(pos.size(), glm::vec3(0.0f));
    for(size_t i=0;i<idx.size(); i+=3){
        glm::vec3 p0 = pos[idx[i+0]];
        glm::vec3 p1 = pos[idx[i+1]];
        glm::vec3 p2 = pos[idx[i+2]];
        glm::vec3 n = glm::normalize(glm::cross(p1-p0,p2-p0));
        normals[idx[i+0]] += n; normals[idx[i+1]] += n; normals[idx[i+2]] += n;
    }
    for(auto &n : normals) n = glm::normalize(n);

    // interleaved buffer pos + normal
    std::vector<float> vbuf;
    for(size_t i=0;i<pos.size();++i){ vbuf.push_back(pos[i].x); vbuf.push_back(pos[i].y); vbuf.push_back(pos[i].z); vbuf.push_back(normals[i].x); vbuf.push_back(normals[i].y); vbuf.push_back(normals[i].z); }

    GLuint VAO,VBO,EBO; glGenVertexArrays(1,&VAO); glGenBuffers(1,&VBO); glGenBuffers(1,&EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO); glBufferData(GL_ARRAY_BUFFER,vbuf.size()*sizeof(float),vbuf.data(),GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO); glBufferData(GL_ELEMENT_ARRAY_BUFFER,idx.size()*sizeof(unsigned int),idx.data(),GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(3*sizeof(float))); glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // compile gouraud program
    GLuint gvs = glCreateShader(GL_VERTEX_SHADER); glShaderSource(gvs,1,&gouraud_vs,nullptr); glCompileShader(gvs); checkCompile(gvs,"g_vs");
    GLuint gfs = glCreateShader(GL_FRAGMENT_SHADER); glShaderSource(gfs,1,&gouraud_fs,nullptr); glCompileShader(gfs); checkCompile(gfs,"g_fs");
    GLuint gouraudProg = glCreateProgram(); glAttachShader(gouraudProg,gvs); glAttachShader(gouraudProg,gfs); glLinkProgram(gouraudProg); checkLink(gouraudProg);
    glDeleteShader(gvs); glDeleteShader(gfs);

    // compile phong program
    GLuint pvs = glCreateShader(GL_VERTEX_SHADER); glShaderSource(pvs,1,&phong_vs,nullptr); glCompileShader(pvs); checkCompile(pvs,"p_vs");
    GLuint pfs = glCreateShader(GL_FRAGMENT_SHADER); glShaderSource(pfs,1,&phong_fs,nullptr); glCompileShader(pfs); checkCompile(pfs,"p_fs");
    GLuint phongProg = glCreateProgram(); glAttachShader(phongProg,pvs); glAttachShader(phongProg,pfs); glLinkProgram(phongProg); checkLink(phongProg);
    glDeleteShader(pvs); glDeleteShader(pfs);

    // common uniforms
    glm::vec3 lightPosWorld = glm::vec3(0.0f,0.0f,1.5f); // near front face
    glm::vec3 viewPosWorld = glm::vec3(0.0f,0.0f,3.0f);
    float shininess = 64.0f;
    float kq = 0.01f;

    glm::mat4 view = glm::translate(glm::mat4(1.0f), -viewPosWorld);
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f/600.0f, 0.1f, 100.0f);

    // set initial uniforms for both programs
    glUseProgram(gouraudProg);
    glUniform3fv(glGetUniformLocation(gouraudProg,"lightPos"),1,glm::value_ptr(lightPosWorld));
    glUniform3fv(glGetUniformLocation(gouraudProg,"viewPos"),1,glm::value_ptr(viewPosWorld));
    glUniform1f(glGetUniformLocation(gouraudProg,"shininess"), shininess);
    glUniform1f(glGetUniformLocation(gouraudProg,"kq"), kq);
    glUniformMatrix4fv(glGetUniformLocation(gouraudProg,"view"),1,GL_FALSE,glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(gouraudProg,"projection"),1,GL_FALSE,glm::value_ptr(proj));

    glUseProgram(phongProg);
    glUniform3fv(glGetUniformLocation(phongProg,"lightPos"),1,glm::value_ptr(lightPosWorld));
    glUniform3fv(glGetUniformLocation(phongProg,"viewPos"),1,glm::value_ptr(viewPosWorld));
    glUniform1f(glGetUniformLocation(phongProg,"shininess"), shininess);
    glUniform1f(glGetUniformLocation(phongProg,"kq"), kq);
    glUniformMatrix4fv(glGetUniformLocation(phongProg,"view"),1,GL_FALSE,glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(phongProg,"projection"),1,GL_FALSE,glm::value_ptr(proj));

    std::cout << "Shading: Gouraud\n";

    while(!glfwWindowShouldClose(win)){
        if(glfwGetKey(win, GLFW_KEY_ESCAPE)==GLFW_PRESS) glfwSetWindowShouldClose(win, GLFW_TRUE);

        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        float t = (float)glfwGetTime();
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), t*glm::radians(25.0f), glm::vec3(1.0f,1.0f,0.0f));

        if(shading==ShadingMode::Gouraud){
            glUseProgram(gouraudProg);
            glUniformMatrix4fv(glGetUniformLocation(gouraudProg,"model"),1,GL_FALSE,glm::value_ptr(model));
        } else {
            glUseProgram(phongProg);
            glUniformMatrix4fv(glGetUniformLocation(phongProg,"model"),1,GL_FALSE,glm::value_ptr(model));
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)idx.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glDeleteProgram(gouraudProg); glDeleteProgram(phongProg);
    glDeleteBuffers(1,&VBO); glDeleteBuffers(1,&EBO); glDeleteVertexArrays(1,&VAO);
    glfwDestroyWindow(win); glfwTerminate();
    return 0;
}
