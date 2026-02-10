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
    lib/imgui/backends/imgui_impl_glfw.cpp
    lib/imgui/backends/imgui_impl_opengl3.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/imgui.cpp
    src/AnimationController.cpp
    src/Camera.cpp
    src/ClippingUtils.cpp
    src/glad.cpp
    src/lodepng.cpp
    src/main.cpp
    src/MenuController.cpp
    src/PickingUtils.cpp
    src/SceneContext.cpp
    src/ScreenshotUtils.cpp
    src/Shader.cpp
    src/TreeRenderer.cpp
    src/VtkReader.cpp
)

add_executable(ArterialVis ${SOURCES})

target_link_libraries(ArterialVis PRIVATE
    glfw
    ${OPENGL_LIBRARIES}
    glm::glm
    ${CMAKE_DL_LIBS}
)
