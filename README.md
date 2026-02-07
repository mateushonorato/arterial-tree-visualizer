# Arterial Tree Visualizer (CCO)

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)]()
[![OpenGL 3.3](https://img.shields.io/badge/OpenGL-3.3-green.svg)]()
[![MIT License](https://img.shields.io/badge/license-MIT-yellow.svg)]()

Visualizador gráfico de alta performance para árvores arteriais geradas pelo método CCO (Constrained Constructive Optimization). Suporta visualização de modelos 2D e 3D baseados em arquivos VTK (Legacy). Implementação original para fins avaliativos na disciplina BCC327 - Computação Gráfica (UFOP).

## Features

- Leitura de arquivos VTK (Legacy)
- Visualização 2D/3D
- Controle de Câmera
- Iluminação (Phong/Gouraud)

## Dependências

- CMake
- GCC/Clang (C++17)
- GLFW3
- GLM

## Instalação e Compilação

### Ubuntu/Debian

```sh
sudo apt install build-essential cmake libglfw3-dev libglm-dev
```

### Arch Linux

```sh
sudo pacman -S base-devel cmake glfw-wayland glm
```

### Build

```sh
mkdir build
cd build
cmake ..
make
./ArterialVis
```
