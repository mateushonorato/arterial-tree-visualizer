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
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui)
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui/backends)

set(SOURCES
    src/main.cpp
    src/glad.c
    src/VtkReader.cpp
    src/shader.cpp
    src/TreeRenderer.cpp
    lib/imgui/imgui.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/backends/imgui_impl_glfw.cpp
    lib/imgui/backends/imgui_impl_opengl3.cpp
)

add_executable(ArterialVis ${SOURCES})

target_link_libraries(ArterialVis PRIVATE
    glfw
    ${OPENGL_LIBRARIES}
    glm::glm
    ${CMAKE_DL_LIBS}
)
