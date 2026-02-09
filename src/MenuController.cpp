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
    
    // Play/Pause button and SPACE shortcut
    if (ImGui::Button(animCtrl.isPlaying() ? "Pause ||" : "Play >") || ImGui::IsKeyPressed(ImGuiKey_Space)) {
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
    dirty |= ImGui::SliderFloat3("Posição da Luz", animCtrl.lightPos, -20.0f, 20.0f);
    dirty |= ImGui::SliderFloat("Transparência", &animCtrl.transparency, 0.0f, 1.0f);
    if (ImGui::RadioButton("Phong (Padrão)", animCtrl.lightingMode == 0)) animCtrl.lightingMode = 0;
    ImGui::SameLine();
    if (ImGui::RadioButton("Gouraud", animCtrl.lightingMode == 1)) animCtrl.lightingMode = 1;
    ImGui::SameLine();
    if (ImGui::RadioButton("Flat", animCtrl.lightingMode == 2)) animCtrl.lightingMode = 2;
    if (ImGui::Checkbox("Mostrar Grade", &animCtrl.showGrid)) {}
    ImGui::SameLine();
    if (ImGui::Checkbox("Mostrar Gizmo", &animCtrl.showGizmo)) {}
    ImGui::Checkbox("Projeção Ortográfica", &animCtrl.useOrthographic);
    if (ImGui::Checkbox("Suavizar Conexões", &animCtrl.showSpheres)) {
        animCtrl.m_visualDirty = true;
    }
    if (dirty) animCtrl.m_visualDirty = true;
    ImGui::End();

    // --- Selection Properties Panel ---
    int selIdx = animCtrl.getSelectedSegment();
    if (selIdx != -1 && selIdx < (int)tree.segments.size()) {
        const auto& seg = tree.segments[selIdx];
        const auto& nodeA = tree.nodes[seg.indexA];
        const auto& nodeB = tree.nodes[seg.indexB];
        float length = glm::length(nodeA.position - nodeB.position);
        ImGui::SetNextWindowSize(ImVec2(350, 180), ImGuiCond_Appearing);
        ImGui::Begin("Propriedades da Seleção", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Raio: %.3f mm", seg.radius);
        ImGui::Text("Comprimento: %.3f mm", length);
        ImGui::Separator();
        ImGui::Text("Início: (%.3f, %.3f, %.3f)", nodeA.position.x, nodeA.position.y, nodeA.position.z);
        ImGui::Text("Fim:    (%.3f, %.3f, %.3f)", nodeB.position.x, nodeB.position.y, nodeB.position.z);
        ImGui::End();
    }
}