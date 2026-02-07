#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include "VtkReader.hpp"
#include "TreeRenderer.hpp"
#include "imgui.h"

class AnimationController {
private:
    std::string rootPath = "../data/TP1_2D/";
    std::vector<std::string> availableDatasets;
    std::vector<std::string> currentPlaylist;
    int currentDatasetIndex = 0;
    int currentFrameIndex = 0;
    bool isPlaying = false;
    float animationSpeed = 0.1f;
    float timeAccumulator = 0.0f;

    void loadPlaylist(const std::string& folderName);
    void loadCurrentFrame(ArterialTree& tree, TreeRenderer& renderer);

public:
    AnimationController();
    void update(float deltaTime, ArterialTree& tree, TreeRenderer& renderer);
    void renderGUI(ArterialTree& tree, TreeRenderer& renderer);
};
