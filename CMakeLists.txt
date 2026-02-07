cmake_minimum_required(VERSION 3.10)

project(ArterialTreeVis)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Prefer modern GLVND OpenGL libraries (fixes CMP0072 warning)
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCES
    src/main.cpp
    src/glad.c
    src/VtkReader.cpp
)

add_executable(ArterialVis ${SOURCES})

target_link_libraries(ArterialVis PRIVATE
    glfw
    ${OPENGL_LIBRARIES}
    glm::glm
    ${CMAKE_DL_LIBS}
)
