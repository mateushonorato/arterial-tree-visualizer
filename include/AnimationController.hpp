#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include "VtkReader.hpp"
#include "TreeRenderer.hpp"
// Nota: Sem <imgui.h> aqui! Mantendo a lógica pura.

class AnimationController {
private:
    std::string rootPath = "../data/TP1_2D/";
    std::vector<std::string> availableDatasets;
    std::vector<std::string> currentPlaylist;
    int currentDatasetIndex = 0;
    int currentFrameIndex = 0;
    
    // Prefixo m_ para membros privados evita conflitos
    bool m_isPlaying = true; 
    float speedMultiplier = 1.0f;
    float timeAccumulator = 0.0f;

    void loadPlaylist(const std::string& folderName);
    void loadCurrentFrame(ArterialTree& tree, TreeRenderer& renderer);

public:
    AnimationController();
    void update(float deltaTime, ArterialTree& tree, TreeRenderer& renderer);

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