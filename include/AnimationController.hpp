/*
 * Universidade Federal de Ouro Preto - UFOP
 * Departamento de Computação - DECOM
 * Disciplina: BCC327 - Computação Gráfica (2025.2)
 * Professor: Rafael Bonfim
 * Trabalho Prático: Visualizador de Árvores Arteriais (CCO)
 * Arquivo: AnimationController.hpp
 * Autor: Mateus Honorato
 * Data: Fevereiro/2026
 * Descrição:
 * Declara o controlador de animação e estados de reprodução/seleção.
 */

#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <glm/glm.hpp>
#include "VtkReader.hpp"
#include "TreeRenderer.hpp"
// Nota: Sem <imgui.h> aqui! Mantendo a lógica pura.

struct ClippingBox {
    glm::vec3 min = glm::vec3(-2.0f);
    glm::vec3 max = glm::vec3(2.0f);
    bool enabled = false;
};

class AnimationController {
    // Solicitação para reset de câmera
    bool m_cameraResetRequested = false;
    // Estado de seleção para picking
    int selectedSegmentIndex = -1;
    // Flag de requisição de screenshot
    bool m_screenshotRequested = false;
public:
        // Interface de requisição de screenshot
        void requestScreenshot() { m_screenshotRequested = true; }
        bool isScreenshotRequested() const { return m_screenshotRequested; }
        void resetScreenshotRequest() { m_screenshotRequested = false; }
    ClippingBox clipping;
            // --- Picking / Seleção ---
            // Define índice do segmento selecionado (-1 se nenhum)
            void selectSegment(int index, const ArterialTree& tree) {
                selectedSegmentIndex = index;
                if (index >= 0 && index < (int)tree.segments.size()) {
                    lastSelectedMidpoint = tree.segments[index].midpoint;
                }
            }
            // Retorna índice do segmento selecionado (-1 se nenhum)
            int getSelectedSegment() const { return selectedSegmentIndex; }
        // Interface de reset de câmera
        bool shouldResetCamera() const { return m_cameraResetRequested; }
        void ackCameraReset() { m_cameraResetRequested = false; }
        void requestCameraReset() { m_cameraResetRequested = true; }
    enum Mode {
        Mode2D,
        Mode3D,
        ModeWireframe
    };

private:
    std::string currentRootPath;
    std::vector<std::string> availableDatasets;
    std::vector<std::string> currentPlaylist;
    int currentDatasetIndex = 0;
    int currentFrameIndex = 0;
    bool m_isPlaying = true;
    float speedMultiplier = 1.0f;
    float timeAccumulator = 0.0f;
    Mode currentMode = Mode2D;
    glm::vec3 lastSelectedMidpoint = glm::vec3(0.0f);

    void loadPlaylist(const std::string& folderName);
    void loadCurrentFrame(ArterialTree& tree, TreeRenderer& renderer);
    void refreshDatasets(ArterialTree* tree = nullptr, TreeRenderer* renderer = nullptr);

public:
    AnimationController();
    void update(float deltaTime, ArterialTree& tree, TreeRenderer& renderer);

    // Troca de modo
    void setMode2D(ArterialTree* tree = nullptr, TreeRenderer* renderer = nullptr);
    void setMode3D(ArterialTree* tree = nullptr, TreeRenderer* renderer = nullptr);
    void setModeWireframe(ArterialTree* tree = nullptr, TreeRenderer* renderer = nullptr);
    Mode getCurrentMode() const { return currentMode; }

    // Getters e setters para a UI (MenuController)
    const std::vector<std::string>& getAvailableDatasets() const;
    int getCurrentDatasetIndex() const;
    void setDatasetIndex(int index, ArterialTree& tree, TreeRenderer& renderer);
    int getCurrentFrameIndex() const;
    int getTotalFrames() const;
    void setFrameIndex(int index, ArterialTree& tree, TreeRenderer& renderer);
    bool isPlaying() const;
    void togglePlay();
    float& getSpeedMultiplierRef();

    // Variáveis de estado visual
    float radiusScale = 1.0f;
    float lineWidth = 2.0f; // Para modo Wireframe (linhas)
    float objectColor[3] = {1.0f, 0.0f, 0.0f}; // Vermelho
    float lightPos[3] = {10.0f, 10.0f, 10.0f};
    float transparency = 1.0f;
    int lightingMode = 0; // 0=Phong, 1=Gouraud, 2=Flat
    bool showGrid = true;
    bool showGizmo = true;
    bool useOrthographic = false;
    bool m_visualDirty = true;
    // Toggle de UI para mostrar esferas (junções)
    bool showSpheres = true;

    bool isVisualDirty() const { return m_visualDirty; }
    void resetVisualDirty() { m_visualDirty = false; }
};