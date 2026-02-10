# Arterial Tree Visualizer (CCO) 🩸

![C++](https://img.shields.io/badge/std-C%2B%2B17-blue.svg?style=flat&logo=c%2B%2B)
![OpenGL](https://img.shields.io/badge/OpenGL-3.3%20Core-green.svg?style=flat&logo=opengl)
![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows-lightgrey)
![License](https://img.shields.io/badge/License-MIT-yellow)

> **Visualizador 3D de alta performance para estruturas vasculares fisiológicas.**
> </br>Projeto desenvolvido como parte da disciplina de Computação Gráfica na UFOP (2025/2).

Este projeto é um renderizador interativo capaz de processar e visualizar a evolução temporal de árvores arteriais geradas pelo algoritmo **CCO (Constrained Constructive Optimization)**. Foi construído do zero utilizando **Modern OpenGL (Programmable Pipeline)**, focando em performance, arquitetura limpa e precisão geométrica.

---

## 📸 Demonstração

![Demo Principal](docs/demo_main.gif)

---

## ✨ Funcionalidades Técnicas

Este visualizador implementa algoritmos geométricos fundamentais "na unha" (sem engines prontas):

### 1. Renderização Avançada (Shaders)
Utilização de **GLSL** customizado para implementar diferentes modelos de iluminação em tempo real:
* **Phong Shading:** Iluminação per-fragment para acabamento realista.
* **Gouraud Shading:** Iluminação per-vertex para otimização.
* **Flat & Wireframe:** Modos de depuração de malha e topologia.

![Comparativo de Shaders](docs/shaders_showcase.png)

### 2. Algoritmo de Recorte (Liang-Barsky)
Implementação manual do algoritmo de **Liang-Barsky** para recorte paramétrico de segmentos.
* **Diferencial:** Ao contrário de técnicas simples de descarte de pixels, este algoritmo recalcula matematicamente os vértices da geometria na CPU, garantindo que a seção transversal dos vasos cortados permaneça visualmente correta.

![Recorte Paramétrico](docs/clipping_algo.png)

### 3. Interação e Ray Casting
Sistema de seleção de objetos (Picking) via **Ray Casting**.
* O sistema inverte as matrizes de Projeção e View (`glm::unProject`) para converter o clique 2D do mouse em um raio no espaço 3D.
* Calcula a interseção analítica Raio-Cilindro para selecionar segmentos com precisão de pixel.
* **Análise de Dados:** Exibe métricas como Resistência, Volume e Razão L/D do segmento selecionado.

![Interação e UI](docs/picking_ui.png)

---

## 🛠️ Stack Tecnológico

O projeto foi construído utilizando bibliotecas padrão da indústria para garantir portabilidade e performance:

* **Linguagem:** C++17
* **API Gráfica:** OpenGL 3.3 (Core Profile)
* **Windowing & Input:** [GLFW](https://www.glfw.org/)
* **Loader de Extensões:** [GLAD](https://glad.dav1d.de/)
* **Matemática Vetorial:** [GLM (OpenGL Mathematics)](https://github.com/g-truc/glm)
* **Interface de Usuário (GUI):** [Dear ImGui](https://github.com/ocornut/imgui)
* **Exportação de Imagem:** [LodePNG](https://lodev.org/lodepng/)

## 🎮 Controles

| Entrada | Ação |
| :--- | :--- |
| **Mouse Esq.** | Rotacionar Câmera (Arcball) |
| **Mouse Dir.** | Mover Câmera (Pan) |
| **Scroll** | Zoom In / Out |
| **Clique** | Selecionar Segmento |
| **Teclado [P]** | **Salvar Screenshot (PNG)** |
| **Teclado [Espaço]** | Play / Pause Animação |

---

## 🔧 Como Compilar

O projeto utiliza **CMake** para gerenciamento de build.

### Pré-requisitos (Linux)

**Ubuntu/Debian:**
```bash
sudo apt install build-essential cmake libglfw3-dev libglm-dev

```

**Arch Linux:**

```bash
sudo pacman -S base-devel cmake glfw-wayland glm

```

### Build (Terminal)

```bash
# 1. Clone o repositório
git clone [https://github.com/seu-usuario/arterial-tree-vis.git](https://github.com/seu-usuario/arterial-tree-vis.git)
cd arterial-tree-vis

# 2. Crie a pasta de build
mkdir build && cd build

# 3. Configure e Compile
cmake ..
make

# 4. Execute
./ArterialVis

```

---

## 📚 Créditos e Autoria

**Autor:** Mateus Honorato
</br>**Instituição:** Universidade Federal de Ouro Preto (UFOP)

Este software foi desenvolvido com foco em **boas práticas de engenharia**, utilizando padrões de projeto e estruturas de dados eficientes. Agradecimentos especiais às documentações do [LearnOpenGL](https://learnopengl.com/) e da biblioteca [GLM](https://github.com/g-truc/glm) que serviram de base para a infraestrutura gráfica.