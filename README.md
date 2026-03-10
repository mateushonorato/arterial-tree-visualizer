# Arterial Tree Visualizer (CCO) 🩸

![C++](https://img.shields.io/badge/std-C%2B%2B17-blue.svg?style=flat&logo=c%2B%2B)
![OpenGL](https://img.shields.io/badge/OpenGL-3.3%20Core-green.svg?style=flat&logo=opengl)
![License](https://img.shields.io/badge/License-MIT-yellow)

> **Visualizador 3D de alta performance para estruturas vasculares fisiológicas.**
> </br>Projeto desenvolvido como parte da disciplina de Computação Gráfica na UFOP (2025/2).

Este projeto é um renderizador interativo capaz de processar e visualizar a evolução temporal de árvores arteriais geradas pelo algoritmo **CCO (Constrained Constructive Optimization)**. Foi construído do zero utilizando **Modern OpenGL (Programmable Pipeline)**, focando em performance, arquitetura limpa e precisão geométrica.

---

## 🗂 Estrutura do Repositório

O repositório está dividido em duas frentes de trabalho:

1. **Visualizador Principal (`src/` e `include/`):** O projeto final de visualização de estruturas vasculares com interface gráfica.
2. **Exercícios Práticos (`exercises/`):** Uma coleção de implementações independentes focadas nos algoritmos fundamentais da Computação Gráfica, seguindo especificação dos exercícios contidos nos slides da disciplina. Inclui algoritmos clássicos de recorte (Cohen-Sutherland, Liang-Barsky, Cyrus-Beck, Sutherland-Hodgman e Weiler-Atherton), modelos de sombreamento (Flat, Gouraud, Phong) e remoção de superfícies escondidas (Back-Face Culling).

---

## 📸 Demonstração

---

## ✨ Funcionalidades Técnicas

Este visualizador implementa algoritmos geométricos fundamentais "na unha" (sem engines prontas):

### 1. Renderização Avançada (Shaders)

Utilização de **GLSL** customizado para implementar diferentes modelos de iluminação em tempo real:

* **Phong Shading:** Iluminação per-fragment para acabamento realista.
* **Gouraud Shading:** Iluminação per-vertex para otimização.

### 2. Bibliotecas Utilizadas

* **Matemática Vetorial:** [GLM](https://github.com/g-truc/glm)
* **Contexto e Janela:** [GLFW](https://www.glfw.org/) + [GLAD](https://glad.dav1d.de/)
* **Interface de Usuário (GUI):** [Dear ImGui](https://github.com/ocornut/imgui)
* **Exportação de Imagem:** [LodePNG](https://lodev.org/lodepng/)

---

## 🎮 Controles do Visualizador

| Entrada | Ação |
| --- | --- |
| **Mouse Esq.** | Rotacionar Câmera (Arcball) |
| **Mouse Dir.** | Mover Câmera (Pan) |
| **Scroll** | Zoom In / Out |
| **Clique** | Selecionar Segmento |
| **Teclado [P]** | Salvar Screenshot (PNG) |
| **Teclado [Espaço]** | Play / Pause Animação |

---

## 🔧 Como Compilar

O projeto utiliza **CMake** para gerenciamento de build. Os exercícios e o projeto principal possuem configurações separadas.

### Pré-requisitos (Linux)

**Ubuntu / Debian:**

```bash
sudo apt install build-essential cmake libglfw3-dev libglm-dev
```

**CachyOS / Arch Linux:**

```bash
sudo pacman -S base-devel cmake glfw-wayland glm
```

### Build do Visualizador Principal

É fundamental utilizar a flag `--recursive` durante o clone para garantir que os submódulos (como o Dear ImGui) sejam baixados corretamente.

```bash
# 1. Clone o repositório baixando também os submódulos
git clone --recursive https://github.com/mateushonorato/arterial-tree-visualizer.git
cd arterial-tree-visualizer

# 2. Crie a pasta de build
mkdir build && cd build

# 3. Configure e Compile
cmake ..
make

# 4. Execute
./ArterialVis
```

### Build dos Exercícios Práticos

Os exercícios da disciplina foram isolados e utilizam um `CMakeLists.txt` próprio localizado na pasta `exercises/`.

```bash
# 1. Entre no diretório de exercícios
cd arterial-tree-visualizer/exercises

# 2. Crie a pasta de build local
mkdir build && cd build

# 3. Configure e Compile
cmake ..
make

# 4. Execute o exercício desejado (exemplo: Algoritmo de Weiler-Atherton)
./ex20_weiler_atherton
```

---

## 📚 Créditos e Autoria

**Autor:** Mateus Honorato</br>
**Instituição:** Universidade Federal de Ouro Preto (UFOP) - Departamento de Computação (DECOM)</br>
**Disciplina:** BCC327 - Computação Gráfica (2025/2)</br>
**Professor:** Rafael Bonfim