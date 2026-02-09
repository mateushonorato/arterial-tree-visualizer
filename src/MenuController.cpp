#include "MenuController.hpp"
#include <imgui.h>
#include <vector>
#include <string>

void MenuController::render(AnimationController& animCtrl, ArterialTree& tree, TreeRenderer& renderer, bool hideMainPanel) {
    if (ImGui::CollapsingHeader("Ferramenta de Corte (Slicing)", ImGuiTreeNodeFlags_DefaultOpen)) {
        bool clippingChanged = false;
        clippingChanged |= ImGui::Checkbox("Ativar Corte", &animCtrl.clipping.enabled);
        ImGui::SameLine();
        if (ImGui::Button("Resetar ##clip")) {
            animCtrl.clipping.min = glm::vec3(-2.0f);
            animCtrl.clipping.max = glm::vec3(2.0f);
            clippingChanged = true;
        }
        if (animCtrl.clipping.enabled) {
            ImGui::Separator();
            auto drawAxisControl = [&](const char* label, float* minVal, float* maxVal) {
                ImGui::Text("%s", label);
                clippingChanged |= ImGui::SliderFloat((std::string("Min ##") + label).c_str(), minVal, -2.0f, 2.0f);
                clippingChanged |= ImGui::SliderFloat((std::string("Max ##") + label).c_str(), maxVal, -2.0f, 2.0f);
                if (*minVal > *maxVal) *minVal = *maxVal;
            };
            bool isRotated3D = (animCtrl.getCurrentMode() == AnimationController::Mode3D);
            drawAxisControl("Eixo X (Largura)", &animCtrl.clipping.min.x, &animCtrl.clipping.max.x);
            ImGui::Spacing();
            // Contextual mapping for Y/Z axes
            if (isRotated3D) {
                drawAxisControl("Eixo Y (Altura)", &animCtrl.clipping.min.z, &animCtrl.clipping.max.z);
                ImGui::Spacing();
                drawAxisControl("Eixo Z (Profundidade)", &animCtrl.clipping.min.y, &animCtrl.clipping.max.y);
            } else {
                drawAxisControl("Eixo Y (Altura)", &animCtrl.clipping.min.y, &animCtrl.clipping.max.y);
                ImGui::Spacing();
                drawAxisControl("Eixo Z (Profundidade)", &animCtrl.clipping.min.z, &animCtrl.clipping.max.z);
            }
        }
        if (clippingChanged) animCtrl.m_visualDirty = true;
    }

    if (!hideMainPanel) {
        ImGui::SetNextWindowSize(ImVec2(480, 250), ImGuiCond_FirstUseEver);
        ImGui::Begin("Menu Principal");

        // --- 2D/3D + Wireframe UI ---
        int mode2D = (animCtrl.getCurrentMode() == AnimationController::Mode2D || animCtrl.getCurrentMode() == AnimationController::ModeWireframe) ? 1 : 0;
        int mode3D = (animCtrl.getCurrentMode() == AnimationController::Mode3D) ? 1 : 0;
        ImGui::Text("Modo de Visualização:");
        ImGui::SameLine();
        if (ImGui::RadioButton("Modo 2D", mode2D)) {
            if (!mode2D) animCtrl.setMode2D(&tree, &renderer);
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Modo 3D", mode3D)) {
            if (!mode3D) animCtrl.setMode3D(&tree, &renderer);
        }
        // Wireframe checkbox only if 2D is active
        if (animCtrl.getCurrentMode() == AnimationController::Mode2D || animCtrl.getCurrentMode() == AnimationController::ModeWireframe) {
            bool wireframeActive = (animCtrl.getCurrentMode() == AnimationController::ModeWireframe);
            if (ImGui::Checkbox("Visualizar Esqueleto (Wireframe)", &wireframeActive)) {
                if (wireframeActive) animCtrl.setModeWireframe(&tree, &renderer);
                else animCtrl.setMode2D(&tree, &renderer);
            }
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

        // --- Common Visual Controls ---
        // Unified Thickness Slider
        // Unified thickness and transparency for all modes
        if (animCtrl.getCurrentMode() == AnimationController::ModeWireframe) {
            dirty |= ImGui::SliderFloat("Espessura", &animCtrl.lineWidth, 1.0f, 8.0f, "%.1f", ImGuiSliderFlags_None);
        } else {
            dirty |= ImGui::SliderFloat("Espessura", &animCtrl.radiusScale, 0.1f, 5.0f, "%.2f", ImGuiSliderFlags_None);
        }
        dirty |= ImGui::SliderFloat("Transparência", &animCtrl.transparency, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_None);
        if (ImGui::Checkbox("Mostrar Grade", &animCtrl.showGrid)) {}
        ImGui::SameLine();
        if (ImGui::Checkbox("Mostrar Gizmo", &animCtrl.showGizmo)) {}
        ImGui::Checkbox("Projeção Ortográfica", &animCtrl.useOrthographic);

        // --- Advanced/Lighting Controls (only for 2D/3D, hidden in Wireframe) ---
        if (animCtrl.getCurrentMode() != AnimationController::ModeWireframe) {
            ImGui::Separator();
            ImGui::Text("Opções Avançadas de Renderização");
            // Light position sliders: remap scale so -20 is 0, 0 is 20, 20 is 40
            float lightPosDisplay[3];
            for (int i = 0; i < 3; ++i) lightPosDisplay[i] = animCtrl.lightPos[i] + 20.0f;
            bool lightChanged = false;
            lightChanged |= ImGui::SliderFloat("Luz X (esq-dir)", &lightPosDisplay[0], 0.0f, 40.0f, "%.2f");
            lightChanged |= ImGui::SliderFloat("Luz Y (baixo-cima)", &lightPosDisplay[1], 0.0f, 40.0f, "%.2f");
            lightChanged |= ImGui::SliderFloat("Luz Z (profundidade)", &lightPosDisplay[2], 0.0f, 40.0f, "%.2f");
            if (lightChanged) {
                for (int i = 0; i < 3; ++i) animCtrl.lightPos[i] = lightPosDisplay[i] - 20.0f;
                dirty = true;
            }
            // Transparência slider is now unified above
            if (ImGui::RadioButton("Phong (Padrão)", animCtrl.lightingMode == 0)) animCtrl.lightingMode = 0;
            ImGui::SameLine();
            if (ImGui::RadioButton("Gouraud", animCtrl.lightingMode == 1)) animCtrl.lightingMode = 1;
            ImGui::SameLine();
            if (ImGui::RadioButton("Flat", animCtrl.lightingMode == 2)) animCtrl.lightingMode = 2;
            if (ImGui::Checkbox("Suavizar Conexões", &animCtrl.showSpheres)) {
                animCtrl.m_visualDirty = true;
            }
        }
        if (dirty) animCtrl.m_visualDirty = true;

        ImGui::Separator();
        // Screenshot Button
        if (ImGui::Button("Salvar PNG (Transparente)")) {
            animCtrl.requestScreenshot();
        }

        ImGui::End();
    }

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