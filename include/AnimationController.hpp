#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include "VtkReader.hpp"
#include "TreeRenderer.hpp"
// Nota: Sem <imgui.h> aqui! Mantendo a lógica pura.

class AnimationController {
    // Camera reset logic
    bool m_cameraResetRequested = false;
    // Selection state for picking
    int selectedSegmentIndex = -1;
public:
            // --- Picking/Selection ---
            // Set selected segment index (-1 for none)
            void selectSegment(int index, const ArterialTree& tree) {
                selectedSegmentIndex = index;
                if (index >= 0 && index < (int)tree.segments.size()) {
                    lastSelectedMidpoint = tree.segments[index].midpoint;
                }
            }
            // Get selected segment index (-1 for none)
            int getSelectedSegment() const { return selectedSegmentIndex; }
        // Camera reset interface
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

    // Mode switching
    void setMode2D(ArterialTree* tree = nullptr, TreeRenderer* renderer = nullptr);
    void setMode3D(ArterialTree* tree = nullptr, TreeRenderer* renderer = nullptr);
    void setModeWireframe(ArterialTree* tree = nullptr, TreeRenderer* renderer = nullptr);
    Mode getCurrentMode() const { return currentMode; }

    // Getters e Setters para a UI (MenuController)
    const std::vector<std::string>& getAvailableDatasets() const;
    int getCurrentDatasetIndex() const;
    void setDatasetIndex(int index, ArterialTree& tree, TreeRenderer& renderer);
    int getCurrentFrameIndex() const;
    int getTotalFrames() const;
    void setFrameIndex(int index, ArterialTree& tree, TreeRenderer& renderer);
    bool isPlaying() const;
    void togglePlay();
    float& getSpeedMultiplierRef();

    // Visual state variables
    float radiusScale = 1.0f;
    float lineWidth = 2.0f; // For Wireframe mode (lines)
    float objectColor[3] = {1.0f, 0.0f, 0.0f}; // Red
    float lightPos[3] = {10.0f, 10.0f, 10.0f};
    float transparency = 1.0f;
    int lightingMode = 0; // 0=Phong, 1=Gouraud, 2=Flat
    bool showGrid = true;
    bool showGizmo = true;
    bool useOrthographic = false;
    bool m_visualDirty = true;
    // UI toggle for showing spheres (joints)
    bool showSpheres = true;

    bool isVisualDirty() const { return m_visualDirty; }
    void resetVisualDirty() { m_visualDirty = false; }
};