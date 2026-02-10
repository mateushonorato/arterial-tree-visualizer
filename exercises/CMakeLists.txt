cmake_minimum_required(VERSION 3.10)
project(CG_Exercises)

set(CMAKE_CXX_STANDARD 17)

# Caminhos baseados na estrutura do seu projeto principal (um nível acima)
set(MAIN_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)
set(MAIN_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../lib)
# Caso seu GLAD.cpp esteja na raiz ou em src do projeto principal:
set(GLAD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../src/glad.cpp) 

include_directories(${MAIN_INC_DIR})

# Localizar GLFW (usando a que você já instalou no sistema para o trabalho)
find_package(glfw3 REQUIRED)

# --- Lista de Exercícios ---
# Para cada novo exercício, adicionaremos uma linha aqui.

# Exercício 03
add_executable(ex03_input ex03_input.cpp ${GLAD_PATH})
target_link_libraries(ex03_input glfw)

# Exercício 10
add_executable(ex10_flashlight ex10_flashlight.cpp ${GLAD_PATH})
target_link_libraries(ex10_flashlight glfw)

# Exercício 12
add_executable(ex12_vertex_normals ex12_vertex_normals.cpp ${GLAD_PATH})
target_link_libraries(ex12_vertex_normals glfw)

# Exercício 13
add_executable(ex13_phong_specular ex13_phong_specular.cpp ${GLAD_PATH})
target_link_libraries(ex13_phong_specular glfw)

# Exercício 04
add_executable(ex04_soma_cores ex04_soma_cores.cpp ${GLAD_PATH})
target_link_libraries(ex04_soma_cores glfw)

# Exercício 05
add_executable(ex05_primitives ex05_primitives.cpp ${GLAD_PATH})
target_link_libraries(ex05_primitives glfw)

# Exercício 06
add_executable(ex06_cube ex06_cube.cpp ${GLAD_PATH})
target_link_libraries(ex06_cube glfw)

# Exercício 09
add_executable(ex09_projection ex09_projection.cpp ${GLAD_PATH})
target_link_libraries(ex09_projection glfw)

# Exercício 20 - Weiler-Atherton prototype
add_executable(ex20_weiler_atherton ex20_weiler_atherton.cpp ${GLAD_PATH})
target_link_libraries(ex20_weiler_atherton glfw)

# Exercício 21 - Culling
add_executable(ex21_culling ex21_culling.cpp ${GLAD_PATH})
target_link_libraries(ex21_culling glfw)

# Exercício 18
add_executable(ex18_cyrus_beck ex18_cyrus_beck.cpp ${GLAD_PATH})
target_link_libraries(ex18_cyrus_beck glfw)