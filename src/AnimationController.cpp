#include "AnimationController.hpp"
#include <algorithm>

AnimationController::AnimationController() {
    // Scan rootPath for dataset folders
    for (const auto& entry : std::filesystem::directory_iterator(rootPath)) {
        if (entry.is_directory()) {
            availableDatasets.push_back(entry.path().filename().string());
        }
    }
    if (!availableDatasets.empty()) {
        loadPlaylist(availableDatasets[0]);
    }
}

void AnimationController::loadPlaylist(const std::string& folderName) {
    currentPlaylist.clear();
    std::string folderPath = rootPath + folderName + "/";
    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".vtk") {
            currentPlaylist.push_back(entry.path().string());
        }
    }
    std::sort(currentPlaylist.begin(), currentPlaylist.end());
    currentFrameIndex = 0;
    loadCurrentFrame(*(new ArterialTree), *(new TreeRenderer)); // Dummy, will be replaced in update/renderGUI
}

void AnimationController::loadCurrentFrame(ArterialTree& tree, TreeRenderer& renderer) {
    if (currentPlaylist.empty()) return;
    VtkReader::load(currentPlaylist[currentFrameIndex], tree);
    renderer.init(tree);
}

void AnimationController::update(float deltaTime, ArterialTree& tree, TreeRenderer& renderer) {
    if (isPlaying && !currentPlaylist.empty()) {
        timeAccumulator += deltaTime;
        if (timeAccumulator >= animationSpeed) {
            timeAccumulator = 0.0f;
            currentFrameIndex++;
            if (currentFrameIndex >= (int)currentPlaylist.size()) {
                currentFrameIndex = 0;
            }
            loadCurrentFrame(tree, renderer);
        }
    }
}

void AnimationController::renderGUI(ArterialTree& tree, TreeRenderer& renderer) {
    ImGui::Begin("Controle de Animação");
    ImGui::Text("Dataset:");
    if (ImGui::BeginCombo("##dataset", availableDatasets.empty() ? "" : availableDatasets[currentDatasetIndex].c_str())) {
        for (int i = 0; i < (int)availableDatasets.size(); ++i) {
            bool isSelected = (currentDatasetIndex == i);
            if (ImGui::Selectable(availableDatasets[i].c_str(), isSelected)) {
                currentDatasetIndex = i;
                loadPlaylist(availableDatasets[i]);
                loadCurrentFrame(tree, renderer);
            }
            if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::Separator();
    ImGui::Text("Frame: %d / %d", currentFrameIndex + 1, (int)currentPlaylist.size());
    if (ImGui::Button(isPlaying ? "Pause" : "Play")) isPlaying = !isPlaying;
    ImGui::SameLine();
    if (ImGui::Button("Stop")) {
        isPlaying = false;
        currentFrameIndex = 0;
        loadCurrentFrame(tree, renderer);
    }
    ImGui::SliderFloat("Velocidade (s)", &animationSpeed, 0.01f, 1.0f);
    int maxFrame = currentPlaylist.empty() ? 0 : (int)currentPlaylist.size() - 1;
    if (ImGui::SliderInt("Timeline", &currentFrameIndex, 0, maxFrame)) {
        isPlaying = false;
        loadCurrentFrame(tree, renderer);
    }
    ImGui::End();
}
