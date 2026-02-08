#include "MenuController.hpp"
#include <imgui.h>
#include <vector>
#include <string>

void MenuController::render(AnimationController& animCtrl, ArterialTree& tree, TreeRenderer& renderer) {
    ImGui::SetNextWindowSize(ImVec2(480, 250), ImGuiCond_FirstUseEver);
    ImGui::Begin("Menu Principal");

    // 1. Seletor de Dataset
    const std::vector<std::string>& datasets = animCtrl.getAvailableDatasets();
    int currentIdx = animCtrl.getCurrentDatasetIndex();
    
    // Proteção contra datasets vazios
    const char* previewValue = (datasets.empty() || currentIdx >= datasets.size()) ? 
                               "Nenhum" : datasets[currentIdx].c_str();

    ImGui::Text("Dataset:");
    if (ImGui::BeginCombo("##dataset", previewValue)) {
        for (int i = 0; i < (int)datasets.size(); i++) {
            bool isSelected = (currentIdx == i);
            if (ImGui::Selectable(datasets[i].c_str(), isSelected)) {
                animCtrl.setDatasetIndex(i, tree, renderer);
            }
            if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::Separator();

    // 2. Controles de Playback
    int currentFrame = animCtrl.getCurrentFrameIndex();
    int totalFrames = animCtrl.getTotalFrames();
    
    ImGui::Text("Frame: %d / %d", currentFrame + 1, totalFrames);
    
    if (ImGui::Button(animCtrl.isPlaying() ? "Pause ||" : "Play >")) {
        animCtrl.togglePlay();
    }

    // 3. Sliders
    ImGui::SliderFloat("Velocidade (x)", &animCtrl.getSpeedMultiplierRef(), 0.1f, 5.0f);
    
    int maxFrame = (totalFrames > 0) ? totalFrames - 1 : 0;
    int tempFrame = currentFrame;
    
    if (ImGui::SliderInt("Timeline", &tempFrame, 0, maxFrame)) {
        // Se o usuário arrastar o slider, atualizamos o frame manualmente
        animCtrl.setFrameIndex(tempFrame, tree, renderer);
    }

    ImGui::End();
}