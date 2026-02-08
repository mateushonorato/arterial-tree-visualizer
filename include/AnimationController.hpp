#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include "VtkReader.hpp"
#include "TreeRenderer.hpp"
// Nota: Sem <imgui.h> aqui! Mantendo a lógica pura.

class AnimationController {
public:
    enum Mode {
        Mode2D,
        Mode3D
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

    void loadPlaylist(const std::string& folderName);
    void loadCurrentFrame(ArterialTree& tree, TreeRenderer& renderer);
    void refreshDatasets(ArterialTree* tree = nullptr, TreeRenderer* renderer = nullptr);

public:
    AnimationController();
    void update(float deltaTime, ArterialTree& tree, TreeRenderer& renderer);

    // Mode switching
    void setMode2D(ArterialTree* tree = nullptr, TreeRenderer* renderer = nullptr);
    void setMode3D(ArterialTree* tree = nullptr, TreeRenderer* renderer = nullptr);
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
};