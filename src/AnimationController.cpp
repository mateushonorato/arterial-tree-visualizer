#include "AnimationController.hpp"
#include <algorithm>
#include <iostream>

AnimationController::AnimationController() {
    m_isPlaying = true;
    radiusScale = 1.0f;
    lightPos[0] = -20.0f;
    lightPos[1] = -20.0f;
    lightPos[2] = -20.0f;
    transparency = 1.0f;
    m_visualDirty = true;
    showSpheres = true;
    selectedSegmentIndex = -1;
    setMode2D();
    requestCameraReset();
}
// --- Mode switching ---
void AnimationController::setMode2D(ArterialTree* tree, TreeRenderer* renderer) {
    currentMode = Mode2D;
    this->selectedSegmentIndex = -1;
    currentRootPath = "../data/TP1_2D/";
    refreshDatasets(tree, renderer);
    requestCameraReset();
}

void AnimationController::setMode3D(ArterialTree* tree, TreeRenderer* renderer) {
    currentMode = Mode3D;
    this->selectedSegmentIndex = -1;
    currentRootPath = "../data/TP2_3D/";
    refreshDatasets(tree, renderer);
    requestCameraReset();
}

void AnimationController::refreshDatasets(ArterialTree* tree, TreeRenderer* renderer) {
    availableDatasets.clear();
    currentPlaylist.clear();
    currentDatasetIndex = 0;
    currentFrameIndex = 0;
    if (std::filesystem::exists(currentRootPath)) {
        for (const auto& entry : std::filesystem::directory_iterator(currentRootPath)) {
            if (entry.is_directory()) {
                availableDatasets.push_back(entry.path().filename().string());
            }
        }
    }
    if (!availableDatasets.empty()) {
        loadPlaylist(availableDatasets[0]);
        if (tree && renderer) {
            loadCurrentFrame(*tree, *renderer);
        }
    } else {
        // Limpa árvore/renderizador se não houver datasets
        if (tree) {
            tree->nodes.clear();
            tree->segments.clear();
        }
        if (renderer && tree) {
            renderer->init(*tree); // Reinitialize with empty tree
        }
    }
}

void AnimationController::loadPlaylist(const std::string& folderName) {
    currentPlaylist.clear();
    std::string folderPath = currentRootPath + folderName + "/";
    
    if (std::filesystem::exists(folderPath)) {
        for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".vtk") {
                currentPlaylist.push_back(entry.path().string());
            }
        }
    }
    
    std::sort(currentPlaylist.begin(), currentPlaylist.end());
    currentFrameIndex = 0;
}

void AnimationController::loadCurrentFrame(ArterialTree& tree, TreeRenderer& renderer) {
    if (currentPlaylist.empty()) return;
    
    if (currentFrameIndex >= 0 && currentFrameIndex < (int)currentPlaylist.size()) {
        if (!VtkReader::load(currentPlaylist[currentFrameIndex], tree)) {
             std::cerr << "Failed to load frame: " << currentPlaylist[currentFrameIndex] << std::endl;
        } else {
            renderer.init(tree, radiusScale);
            // Restore persistent selection if lastSelectedMidpoint is valid
            if (selectedSegmentIndex != -1 && tree.segments.size() > 0) {
                float minDist = std::numeric_limits<float>::max();
                int bestIdx = -1;
                for (size_t i = 0; i < tree.segments.size(); ++i) {
                    float dist = glm::distance(tree.segments[i].midpoint, lastSelectedMidpoint);
                    if (dist < minDist) {
                        minDist = dist;
                        bestIdx = static_cast<int>(i);
                    }
                }
                selectedSegmentIndex = bestIdx;
            }
        }
    }
}

void AnimationController::update(float deltaTime, ArterialTree& tree, TreeRenderer& renderer) {
    // Garante carregamento inicial se a árvore estiver vazia e tivermos arquivos
    if (tree.nodes.empty() && !currentPlaylist.empty()) {
        loadCurrentFrame(tree, renderer);
    }

    const float baseDelay = 0.1f;
    // Usa m_isPlaying aqui
    if (m_isPlaying && !currentPlaylist.empty()) {
        float timePerFrame = baseDelay / speedMultiplier;
        timeAccumulator += deltaTime;
        
        if (timeAccumulator >= timePerFrame) {
            timeAccumulator = 0.0f;
            currentFrameIndex++;
            if (currentFrameIndex >= (int)currentPlaylist.size()) {
                currentFrameIndex = 0;
            }
            loadCurrentFrame(tree, renderer);
        }
    }
}

// Implementação dos Getters/Setters
const std::vector<std::string>& AnimationController::getAvailableDatasets() const {
    return availableDatasets;
}

int AnimationController::getCurrentDatasetIndex() const {
    return currentDatasetIndex;
}

void AnimationController::setDatasetIndex(int index, ArterialTree& tree, TreeRenderer& renderer) {
    if (index >= 0 && index < (int)availableDatasets.size()) {
        currentDatasetIndex = index;
        this->selectedSegmentIndex = -1;
        loadPlaylist(availableDatasets[index]);
        loadCurrentFrame(tree, renderer);
        requestCameraReset();
    }
}

int AnimationController::getCurrentFrameIndex() const {
    return currentFrameIndex;
}

int AnimationController::getTotalFrames() const {
    return (int)currentPlaylist.size();
}

void AnimationController::setFrameIndex(int index, ArterialTree& tree, TreeRenderer& renderer) {
    if (index >= 0 && index < (int)currentPlaylist.size()) {
        currentFrameIndex = index;
        m_isPlaying = false; // Pausa se o usuário mexer na timeline
        loadCurrentFrame(tree, renderer);
    }
}

bool AnimationController::isPlaying() const {
    return m_isPlaying;
}

void AnimationController::togglePlay() {
    m_isPlaying = !m_isPlaying;
}

float& AnimationController::getSpeedMultiplierRef() {
    return speedMultiplier;
}