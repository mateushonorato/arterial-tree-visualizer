#include "MenuController.hpp"
#include <imgui.h>
#include <vector>
#include <string>

void MenuController::render(AnimationController& animCtrl, ArterialTree& tree, TreeRenderer& renderer) {
    ImGui::SetNextWindowSize(ImVec2(480, 250), ImGuiCond_FirstUseEver);
    ImGui::Begin("Menu Principal");


    // --- 2D/3D Mode Switch ---
    int mode = (animCtrl.getCurrentMode() == AnimationController::Mode2D) ? 0 : 1;
    ImGui::Text("Modo de Visualização:");
    ImGui::SameLine();
    if (ImGui::RadioButton("Modo 2D", mode == 0)) {
        if (mode != 0) animCtrl.setMode2D(&tree, &renderer);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Modo 3D", mode == 1)) {
        if (mode != 1) animCtrl.setMode3D(&tree, &renderer);
    }
    ImGui::Separator();

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

    ImGui::Separator();
    ImGui::Text("Ajustes Visuais");
    bool dirty = false;
    dirty |= ImGui::SliderFloat("Escala do Raio", &animCtrl.radiusScale, 0.1f, 5.0f);
    dirty |= ImGui::ColorEdit3("Cor do Objeto", animCtrl.objectColor);
    dirty |= ImGui::SliderFloat3("Posição da Luz", animCtrl.lightPos, -20.0f, 20.0f);
    dirty |= ImGui::SliderFloat("Transparência", &animCtrl.transparency, 0.0f, 1.0f);
    if (dirty) animCtrl.m_visualDirty = true;
    ImGui::End();
}