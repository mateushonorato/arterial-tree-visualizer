#include "MenuController.hpp"
#include <imgui.h>
#include <vector>
#include <string>

void MenuController::render(AnimationController &animCtrl, ArterialTree &tree, TreeRenderer &renderer, bool hideMainPanel)
{
    if (!hideMainPanel)
    {
        ImGui::SetNextWindowPos(ImVec2(33, 34), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(598, 697), ImGuiCond_FirstUseEver);
        ImGui::Begin("Menu Principal", nullptr, ImGuiWindowFlags_None);
        // Hide menu only on first frame after startup
        static bool firstFrame = true;
        if (firstFrame && ImGui::IsWindowAppearing())
        {
            ImGui::SetWindowCollapsed(true);
            firstFrame = false;
        }

        // --- Category 1: Modo de Visualização ---
        if (ImGui::CollapsingHeader("Modo de Visualização", ImGuiTreeNodeFlags_DefaultOpen))
        {
            bool modeChanged = false; // Flag para marcar se houve mudança que requer atualização
            int mode2D = (animCtrl.getCurrentMode() == AnimationController::Mode2D || animCtrl.getCurrentMode() == AnimationController::ModeWireframe) ? 1 : 0;
            int mode3D = (animCtrl.getCurrentMode() == AnimationController::Mode3D) ? 1 : 0;
            if (ImGui::RadioButton("Modo 2D", mode2D))
            {
                if (!mode2D)
                {
                    animCtrl.setMode2D(&tree, &renderer);
                    modeChanged = true;
                }
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Modo 3D", mode3D))
            {
                if (!mode3D)
                {
                    animCtrl.setMode3D(&tree, &renderer);
                    modeChanged = true;
                }
            }
            // Wireframe checkbox only if 2D is active
            if (animCtrl.getCurrentMode() == AnimationController::Mode2D || animCtrl.getCurrentMode() == AnimationController::ModeWireframe)
            {
                ImGui::SameLine();
                bool wireframeActive = (animCtrl.getCurrentMode() == AnimationController::ModeWireframe);
                if (ImGui::Checkbox("Visualizar Esqueleto (Wireframe)", &wireframeActive))
                {
                    if (wireframeActive)
                        animCtrl.setModeWireframe(&tree, &renderer);
                    else
                        animCtrl.setMode2D(&tree, &renderer);
                    modeChanged = true;
                }
            }
            // 1. Seletor de Dataset
            const std::vector<std::string> &datasets = animCtrl.getAvailableDatasets();
            int currentIdx = animCtrl.getCurrentDatasetIndex();
            // Proteção contra datasets vazios
            const char *previewValue = (datasets.empty() || currentIdx >= datasets.size()) ? "Nenhum" : datasets[currentIdx].c_str();
            if (ImGui::BeginCombo("Dataset ##dataset", previewValue))
            {
                for (int i = 0; i < (int)datasets.size(); i++)
                {
                    bool isSelected = (currentIdx == i);
                    if (ImGui::Selectable(datasets[i].c_str(), isSelected))
                    {
                        animCtrl.setDatasetIndex(i, tree, renderer);
                        modeChanged = true; // Agora troca de dataset também marca como sujo
                    }
                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            // APLICA O modeChanged SE HOUVE MUDANÇA
            if (modeChanged)
            {
                animCtrl.m_visualDirty = true;
            }
        }

        // --- Category 2: Animação ---
        if (ImGui::CollapsingHeader("Animação", ImGuiTreeNodeFlags_DefaultOpen))
        {
            bool animChanged = false;
            // Frame slider
            int currentFrame = animCtrl.getCurrentFrameIndex();
            int totalFrames = animCtrl.getTotalFrames();
            int maxFrame = (totalFrames > 0) ? totalFrames - 1 : 0;
            if (ImGui::SliderInt("Frame Atual", &currentFrame, 0, maxFrame))
            {
                animCtrl.setFrameIndex(currentFrame, tree, renderer);
                animChanged = true;
            }
            // Play/Pause button only
            ImGui::SameLine();
            bool playPauseClicked = ImGui::Button(animCtrl.isPlaying() ? "Pause ||" : "Play >");
            bool spacePressed = ImGui::IsKeyPressed(ImGuiKey_Space);
            if (playPauseClicked || spacePressed)
            {
                animCtrl.togglePlay();
                animChanged = true;
            }
            // Speed slider
            float speed = animCtrl.getSpeedMultiplierRef();
            if (ImGui::SliderFloat("Velocidade ", &speed, 0.1f, 5.0f))
            {
                animCtrl.getSpeedMultiplierRef() = speed;
                animChanged = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Reset"))
            {
                animCtrl.getSpeedMultiplierRef() = 1.0f;
                animChanged = true;
            }
            // Timeline visualization (keep existing logic if any)
            // ...existing timeline visualization code...
            if (animChanged)
                animCtrl.m_visualDirty = true;
        }

        // --- Category 3: Ajustes Visuais ---
        if (ImGui::CollapsingHeader("Ajustes Visuais", ImGuiTreeNodeFlags_DefaultOpen))
        {
            bool visualChanged = false;
            // Espessura da Linha
            if (animCtrl.getCurrentMode() == AnimationController::ModeWireframe)
            {
                visualChanged |= ImGui::SliderFloat("Espessura da Linha", &animCtrl.lineWidth, 1.0f, 8.0f, "%.1f");
                ImGui::SameLine();
                if (ImGui::Button("Reset##wire"))
                {
                    animCtrl.lineWidth = 2.0f;
                    visualChanged = true;
                }
            }
            else
            {
                visualChanged |= ImGui::SliderFloat("Espessura da Linha", &animCtrl.radiusScale, 0.1f, 5.0f, "%.2f");
                ImGui::SameLine();
                if (ImGui::Button("Reset##radius"))
                {
                    animCtrl.radiusScale = 1.0f;
                    visualChanged = true;
                }
            }
            // Transparência
            visualChanged |= ImGui::SliderFloat("Transparência     ", &animCtrl.transparency, 0.0f, 1.0f, "%.2f");
            ImGui::SameLine();
            if (ImGui::Button("Reset##transp"))
            {
                animCtrl.transparency = 1.0f;
                visualChanged = true;
            }
            // Checkboxes
            visualChanged |= ImGui::Checkbox("Mostrar Grade", &animCtrl.showGrid);
            ImGui::SameLine();
            visualChanged |= ImGui::Checkbox("Mostrar Gizmo", &animCtrl.showGizmo);
            visualChanged |= ImGui::Checkbox("Projeção Ortográfica", &animCtrl.useOrthographic);
            // Suavizar Conexões (moved here)
            ImGui::SameLine();
            visualChanged |= ImGui::Checkbox("Suavizar Conexões", &animCtrl.showSpheres);
            if (visualChanged)
                animCtrl.m_visualDirty = true;
        }

        // --- Category 4: Iluminação ---
        if (animCtrl.getCurrentMode() != AnimationController::ModeWireframe)
        {
            if (ImGui::CollapsingHeader("Iluminação", ImGuiTreeNodeFlags_DefaultOpen))
            {
                bool lightChanged = false;
                // Radio buttons
                ImGui::TextUnformatted("Modelo de Iluminação:                                        ");
                ImGui::SameLine();
                if (ImGui::Button("Resetar Todos##ilum"))
                {
                    animCtrl.lightPos[0] = 20.0f;
                    animCtrl.lightPos[1] = 20.0f;
                    animCtrl.lightPos[2] = 20.0f;
                    lightChanged = true;
                }
                if (ImGui::RadioButton("Phong", animCtrl.lightingMode == 0))
                {
                    animCtrl.lightingMode = 0;
                    lightChanged = true;
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("Gouraud", animCtrl.lightingMode == 1))
                {
                    animCtrl.lightingMode = 1;
                    lightChanged = true;
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("Flat", animCtrl.lightingMode == 2))
                {
                    animCtrl.lightingMode = 2;
                    lightChanged = true;
                }
                // Sliders for light position with Reset buttons
                lightChanged |= ImGui::SliderFloat("Luz X", &animCtrl.lightPos[0], -20.0f, 20.0f);
                ImGui::SameLine();
                if (ImGui::Button("Reset##luzx"))
                {
                    animCtrl.lightPos[0] = 20.0f;
                    lightChanged = true;
                }
                lightChanged |= ImGui::SliderFloat("Luz Y", &animCtrl.lightPos[1], -20.0f, 20.0f);
                ImGui::SameLine();
                if (ImGui::Button("Reset##luzy"))
                {
                    animCtrl.lightPos[1] = 20.0f;
                    lightChanged = true;
                }
                lightChanged |= ImGui::SliderFloat("Luz Z", &animCtrl.lightPos[2], -20.0f, 20.0f);
                ImGui::SameLine();
                if (ImGui::Button("Reset##luzz"))
                {
                    animCtrl.lightPos[2] = 20.0f;
                    lightChanged = true;
                }
                if (lightChanged)
                    animCtrl.m_visualDirty = true;
            }
        }

        // --- Category 5: Ferramentas de Corte ---
        if (ImGui::CollapsingHeader("Ferramentas de Corte", ImGuiTreeNodeFlags_DefaultOpen))
        {
            bool clipChanged = false;
            clipChanged |= ImGui::Checkbox("Ativar Corte                                             ", &animCtrl.clipping.enabled);
            ImGui::SameLine();
            if (ImGui::Button("Resetar Todos##clip"))
            {
                animCtrl.clipping.min.x = -2.0f;
                animCtrl.clipping.max.x = 2.0f;
                animCtrl.clipping.min.y = -2.0f;
                animCtrl.clipping.max.y = 2.0f;
                animCtrl.clipping.min.z = -2.0f;
                animCtrl.clipping.max.z = 2.0f;
                clipChanged = true;
            }

            auto drawAxisControl = [&](const char *label, float *minVal, float *maxVal, float defaultMin, float defaultMax)
            {
                ImGui::Text("%s", label);
                clipChanged |= ImGui::SliderFloat((std::string("Min ##") + label).c_str(), minVal, -2.0f, 2.0f);
                ImGui::SameLine();
                if (ImGui::Button((std::string("Reset##min") + label).c_str()))
                {
                    *minVal = defaultMin;
                    clipChanged = true;
                }
                clipChanged |= ImGui::SliderFloat((std::string("Max ##") + label).c_str(), maxVal, -2.0f, 2.0f);
                ImGui::SameLine();
                if (ImGui::Button((std::string("Reset##max") + label).c_str()))
                {
                    *maxVal = defaultMax;
                    clipChanged = true;
                }
                if (*minVal > *maxVal)
                    *minVal = *maxVal;
            };
            bool isRotated3D = (animCtrl.getCurrentMode() == AnimationController::Mode3D);
            drawAxisControl("Eixo X (Largura)", &animCtrl.clipping.min.x, &animCtrl.clipping.max.x, -2.0f, 2.0f);
            ImGui::Spacing();
            if (isRotated3D)
            {
                drawAxisControl("Eixo Y (Altura)", &animCtrl.clipping.min.z, &animCtrl.clipping.max.z, -2.0f, 2.0f);
                ImGui::Spacing();
                drawAxisControl("Eixo Z (Profundidade)", &animCtrl.clipping.min.y, &animCtrl.clipping.max.y, -2.0f, 2.0f);
            }
            else
            {
                drawAxisControl("Eixo Y (Altura)", &animCtrl.clipping.min.y, &animCtrl.clipping.max.y, -2.0f, 2.0f);
                ImGui::Spacing();
                drawAxisControl("Eixo Z (Profundidade)", &animCtrl.clipping.min.z, &animCtrl.clipping.max.z, -2.0f, 2.0f);
            }

            if (clipChanged)
                animCtrl.m_visualDirty = true;
        }

        // --- Footer: Salvar PNG ---
        ImGui::Separator();
        if (ImGui::Button("Salvar PNG (Transparente)"))
        {
            animCtrl.requestScreenshot();
        }
        ImGui::End();
    }

    int selIdx = animCtrl.getSelectedSegment();
    if (selIdx != -1 && selIdx < (int)tree.segments.size())
    {
        const auto &seg = tree.segments[selIdx];
        const auto &nodeA = tree.nodes[seg.indexA];
        const auto &nodeB = tree.nodes[seg.indexB];

        // --- Cálculos Físicos (Baseado em VTK/Cilindros) ---
        float length = glm::length(nodeA.position - nodeB.position);
        float diameter = seg.radius * 2.0f;
        float area = 3.14159265f * seg.radius * seg.radius;
        float volume = area * length;

        // Resistência Geométrica Proporcional (L / r^4)
        float r4 = std::pow(seg.radius, 4);
        float resistance = (r4 > 1e-8f) ? (length / r4) : 0.0f;

        // Razão de Aspecto
        float aspectRatio = (diameter > 1e-8f) ? (length / diameter) : 0.0f;

        // Renderiza a janela
        ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Appearing); // Auto-resize na primeira vez
        ImGui::Begin("Propriedades da Seleção", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "Geometria do Vaso");
        ImGui::Separator();
        // UNIDADES ADICIONADAS AQUI:
        ImGui::Text("Comprimento: %.4f mm", length);
        ImGui::Text("Raio:        %.4f mm", seg.radius);
        ImGui::Text("Diâmetro:    %.4f mm", diameter);

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Hemodinâmica (Estimada)");
        ImGui::Separator();

        // UNIDADES ADICIONADAS AQUI:
        ImGui::Text("Área Seção:  %.4f mm^2", area);
        ImGui::Text("Volume:      %.4f mm^3", volume);

        // UNIDADES ADICIONADAS AQUI:
        ImGui::Text("Resistência: %.2f mm^-3", resistance);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Resistência Geométrica (L / r^4).\nUnidade: mm^-3");

        ImGui::Text("Razão L/D:   %.2f (adim.)", aspectRatio);

        ImGui::Spacing();
        ImGui::Separator();

        // --- Coordenadas Espaciais ---
        if (ImGui::TreeNode("Coordenadas Espaciais (XYZ)"))
        {
            ImGui::TextDisabled("Nó Inicial (%d):", seg.indexA);
            // UNIDADES ADICIONADAS AQUI:
            ImGui::Text("  (%.3f, %.3f, %.3f) mm", nodeA.position.x, nodeA.position.y, nodeA.position.z);

            ImGui::TextDisabled("Nó Final (%d):", seg.indexB);
            // UNIDADES ADICIONADAS AQUI:
            ImGui::Text("  (%.3f, %.3f, %.3f) mm", nodeB.position.x, nodeB.position.y, nodeB.position.z);

            ImGui::TreePop();
        }

        ImGui::End();
    }
}