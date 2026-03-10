# Arterial Tree Visualizer (CCO) 🩸

![C++](https://img.shields.io/badge/std-C%2B%2B17-blue.svg?style=flat&logo=c%2B%2B)
![OpenGL](https://img.shields.io/badge/OpenGL-3.3%20Core-green.svg?style=flat&logo=opengl)
![License](https://img.shields.io/badge/License-MIT-yellow)

> **Visualizador 3D de alta performance para estruturas vasculares fisiológicas.**
> </br>Projeto desenvolvido como parte da disciplina de Computação Gráfica na UFOP (2025/2).

Este projeto é um renderizador interativo capaz de processar e visualizar a evolução temporal de árvores arteriais geradas pelo algoritmo **CCO (Constrained Constructive Optimization)**. Foi construído do zero utilizando **Modern OpenGL (Programmable Pipeline)**, focando em performance, arquitetura limpa e precisão geométrica.

---

## 📸 Demonstração

---

## 🗂 Estrutura do Repositório

O repositório está dividido em duas frentes de trabalho:

### 1. Visualizador Principal (`src/`, `include/`, `shaders/`)

O projeto final de visualização interativa de estruturas vasculares, composto pelos seguintes módulos-chave:

| Módulo | Arquivo | Responsabilidade |
| --- | --- | --- |
| **Parser VTK** | `VtkReader.cpp` | Leitura e interpretação de arquivos `.vtk` contendo nós, segmentos e raios das árvores arteriais geradas pelo algoritmo CCO. |
| **Modelo de Dados** | `ArterialTree.cpp` | Estruturas `ArterialNode` e `ArterialSegment` com normalização automática (bounding box → volume canônico). |
| **Renderizador** | `TreeRenderer.cpp` | Geração procedural de malhas 3D (cilindros e esferas), wireframe 2D com mapeamento de cores por heat map, e pipeline de buffers VAO/VBO/EBO. |
| **Shaders GLSL** | `vertex.glsl` / `fragment.glsl` | Implementação dos modelos de iluminação Phong, Gouraud e Flat com suporte a destaque de segmentos selecionados e transparência. |
| **Câmera Orbital** | `Camera.cpp` | Câmera Arcball com Euler Angles (Yaw/Pitch), suporte a Pan no espaço da tela e controle de Zoom por distância radial. |
| **Ray Casting (Picking)** | `PickingUtils.cpp` | Seleção 3D de segmentos vasculares via `glm::unProject`, convertendo coordenadas de tela em raios no espaço do mundo para teste de interseção raio-cilindro. |
| **Recorte Geométrico** | `ClippingUtils.cpp` | Recorte paramétrico de segmentos de reta em 3D utilizando o algoritmo de **Liang-Barsky**, permitindo isolar regiões de interesse da árvore arterial. |
| **Interface Gráfica** | `MenuController.cpp` | Painel de controle interativo via Dear ImGui com exibição de propriedades geométricas e hemodinâmicas (comprimento, raio, área, volume, resistência) do segmento selecionado. |
| **Animação** | `AnimationController.cpp` | Controlador de reprodução temporal: carregamento de frames VTK em sequência, playlist de datasets, controle de play/pause e velocidade. |
| **Contexto de Cena** | `SceneContext.cpp` | Desenho da grade de referência (grid) e do gizmo de orientação dos eixos XYZ. |
| **Screenshot** | `ScreenshotUtils.cpp` | Captura do framebuffer OpenGL e exportação para PNG via LodePNG. |
| **Utilitários** | `Shader.cpp` | Compilação, linkagem e gerenciamento de programas GLSL a partir de arquivos em disco. |

### 2. Exercícios Práticos (`exercises/`)

Uma coleção de implementações independentes focadas nos algoritmos fundamentais da Computação Gráfica, seguindo especificação dos exercícios contidos nos slides da disciplina:

| Exercício | Tópico | Descrição |
| --- | --- | --- |
| `ex03_input` | Fundamentos | Tratamento de entrada via teclado (callbacks GLFW). |
| `ex04_soma_cores` | Teoria das Cores | Soma aditiva de cores RGB com uniforms no Fragment Shader. |
| `ex04_teoria` | Teoria das Cores | Conversão RGB → CMYK e justificativa do modelo subtrativo. |
| `ex05_primitives` | Primitivas | Renderização com `GL_TRIANGLE_STRIP` e interpolação de cores. |
| `ex06_cube` | Modelagem 3D | Cubo com rotação animada via matrizes MVP e depth test. |
| `ex08_teoria` | Transformações | Parâmetros de `gluLookAt` e ordem de transformações (T→R) para braço robótico. |
| `ex09_projection` | Projeções | Alternância interativa entre projeção perspectiva e ortográfica. |
| `ex10_flashlight` | Fontes de Luz | Simulação de spotlight (lanterna) com atenuação quadrática. |
| `ex11_teoria` | Iluminação | Análise do comportamento de `GL_SMOOTH` (Gouraud) vs. `GL_FLAT` em quinas. |
| `ex12_vertex_normals` | Gouraud Shading | Sombreamento suave com normais por vértice (média de faces adjacentes). |
| `ex13_phong_specular` | Phong Shading | Comparação Gouraud vs. Phong com componente especular Blinn-Phong. |
| `ex13_teoria` | Phong Shading | Discussão sobre artefatos do destaque especular no Gouraud e necessidade do Phong. |
| `ex18_cyrus_beck` | Recorte de Linhas | Algoritmo de Cyrus-Beck para recorte paramétrico de segmentos. |
| `ex19_sutherland_hodgman` | Recorte de Polígonos | Algoritmo de Sutherland-Hodgman para recorte de polígonos convexos. |
| `ex20_weiler_atherton` | Recorte de Polígonos | Algoritmo de Weiler-Atherton com suporte a polígonos côncavos e buracos. |
| `ex21_culling` | Visibilidade | Remoção de superfícies escondidas via Back-Face Culling. |
| `ex21_teoria` | Visibilidade | Efeito da inversão de `glFrontFace` (CW vs. CCW) no Back-Face Culling. |

### 3. Dados de Simulação (`data/`)

Conjuntos de arquivos `.vtk` gerados pelo algoritmo CCO, organizados por dimensionalidade e número de terminais:

* `TP1_2D/` — Árvores 2D com 64, 128 e 256 terminais.
* `TP2_3D/` — Árvores 3D com 128, 256 e 512 terminais.

---

## ✨ Funcionalidades Técnicas

Este visualizador implementa algoritmos geométricos fundamentais "na unha" (sem engines prontas):

### 1. Renderização Avançada (Shaders GLSL)

Utilização de **GLSL 330 Core** customizado para implementar diferentes modelos de iluminação em tempo real:

* **Phong Shading:** Iluminação per-fragment com componentes ambiente, difusa e especular calculadas no Fragment Shader, proporcionando acabamento realista.
* **Gouraud Shading:** Iluminação per-vertex calculada no Vertex Shader e interpolada via rasterização, priorizando performance.
* **Flat Shading:** Iluminação por face utilizando normais geométricas reconstruídas via `dFdx`/`dFdy` no Fragment Shader, produzindo aspecto facetado.

### 2. Câmera e Transformações

* **Câmera Orbital (Arcball):** Navegação 3D baseada em **Euler Angles (Yaw/Pitch)**, com conversão de coordenadas esféricas para cartesianas, permitindo rotação livre ao redor do modelo.
* **Pan no Espaço da Tela:** Translação da câmera nos eixos locais (X=Direita, Y=Cima) independente da rotação, via multiplicação de matrizes pela esquerda.
* **Múltiplas Matrizes de Transformação:** Composição explícita de matrizes **Model** (rotação do modelo 3D), **View** (`glm::lookAt` + pan offset) e **Projection** (alternância entre perspectiva e ortográfica em tempo real).

### 3. Algoritmos Geométricos

* **Ray Casting para Picking 3D:** Seleção interativa de segmentos vasculares via inversão das matrizes MVP (`glm::unProject`), com teste de interseção raio-cilindro.
* **Recorte de Segmentos (Liang-Barsky 3D):** Recorte paramétrico em caixa delimitadora tri-dimensional para isolar regiões de interesse.
* **Geração Procedural de Malha:** Construção de cilindros e esferas com cálculo de normais para os modelos de iluminação.

### 4. Bibliotecas Utilizadas

| Biblioteca | Finalidade no Projeto |
| --- | --- |
| [GLM](https://github.com/g-truc/glm) | Operações de álgebra linear (vetores, matrizes, transformações) para cálculo das matrizes MVP, normais e `unProject` no sistema de picking. |
| [GLFW](https://www.glfw.org/) + [GLAD](https://glad.dav1d.de/) | Criação da janela OpenGL 3.3 Core Profile, gerenciamento de contexto e carregamento dinâmico das funções OpenGL modernas. |
| [Dear ImGui](https://github.com/ocornut/imgui) | Construção do painel de controle interativo (modos de visualização, animação, iluminação, ferramentas de corte) e exibição em tempo real de dados geométricos e hemodinâmicos do segmento selecionado. |
| [LodePNG](https://lodev.org/lodepng/) | Captura e exportação de screenshots do framebuffer OpenGL diretamente para o formato PNG, sem dependências externas adicionais. |

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

## 📚 Créditos, Referências Bibliográficas e Tutoriais

**Autor:** Mateus Honorato</br>
**Instituição:** Universidade Federal de Ouro Preto (UFOP) - Departamento de Computação (DECOM)</br>
**Disciplina:** BCC327 - Computação Gráfica</br>

Os seguintes recursos foram fundamentais para a fundamentação teórica e a implementação deste projeto:

| Referência | Contribuição |
| --- | --- |
| **[LearnOpenGL.com](https://learnopengl.com/)** — Joey de Vries | Arquitetura base do OpenGL Moderno (Programmable Pipeline): estrutura de inicialização GLFW/GLAD, classe `Camera` com Euler Angles (Yaw/Pitch), compilação de shaders GLSL, alocação de buffers (VAO/VBO/EBO) e implementação do modelo de iluminação de Phong. |
| **[Dear ImGui](https://github.com/ocornut/imgui)** — Omar Cornut | Biblioteca de interface gráfica imediata utilizada para construção de todo o painel de controle interativo, incluindo os exemplos oficiais que serviram de base para a integração com GLFW/OpenGL3. |
| **[LodePNG](https://lodev.org/lodepng/)** — Lode Vandevenne | Biblioteca open-source para codificação PNG utilizada no utilitário de captura de screenshots. |
| **[GLM](https://github.com/g-truc/glm)** — G-Truc Creation | Documentação técnica da biblioteca utilizada como referência para operações vetoriais, matriciais e a função `unProject` no sistema de picking. |
| **Foley, J. D. & van Dam, A.** — *Computer Graphics: Principles and Practice* | Fundamentação matemática dos algoritmos de recorte geométrico (Liang-Barsky, Cyrus-Beck, Sutherland-Hodgman, Weiler-Atherton) implementados nos exercícios práticos e no módulo de clipping do visualizador. |
| **Prof. Rafael Bonfim** — DECOM/UFOP | Conceitos, algoritmos e especificações apresentados nas aulas e slides da disciplina BCC327. |