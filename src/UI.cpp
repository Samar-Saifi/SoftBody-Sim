#include "UI.h"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include "physics/SoftBodySphere.h"
#include "physics/SoftBodyCube.h"

void UI::Initialize(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void UI::BeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UI::Draw(std::unique_ptr<SoftBody>& activeBody) {
    ImGui::Begin("Shape Spawner Control Panel");

    if (!activeBody) {
        ImGui::Text("No active simulation target found.");
        ImGui::End();
        return;
    }

    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Shape Selection");
    int currentShapeIdx = static_cast<int>(m_SelectedShapeType);
    const char* shapeNames[] = { "Sphere", "Cube" };
    if (ImGui::Combo("Shape Type", &currentShapeIdx, shapeNames, IM_ARRAYSIZE(shapeNames))) {
        m_SelectedShapeType = static_cast<ShapeType>(currentShapeIdx);
    }

    ImGui::Separator();

    switch (m_SelectedShapeType) {
        case ShapeType::Sphere:
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Sphere Geometry Config");
            ImGui::SliderFloat("Sphere Radius", &m_SphereConfig.radius, 4.0f, 20.0f);
            ImGui::SliderInt("Mesh Stacks", &m_SphereConfig.stacks, 4, 25);
            ImGui::SliderInt("Mesh Sectors", &m_SphereConfig.sectors, 4, 25);
            break;

        case ShapeType::Cube:
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Cube Geometry Config");
            ImGui::SliderFloat("Cube Size", &m_CubeConfig.size, 2.0f, 20.0f);
            ImGui::SliderInt("Grid Resolution", &m_CubeConfig.resolution, 2, 7);
            break;

        default:
            break;
    }

    ImGui::Spacing();

    if (ImGui::Button("Spawn / Regenerate Shape", ImVec2(-1, 30))) {
        float k1 = activeBody->k_1;
        float k2 = activeBody->k_2;
        float k3 = activeBody->k_3;
        float d1 = activeBody->DAMPING;
        float d2 = activeBody->DAMPING_2;
        float d3 = activeBody->DAMPING_3;
        int subs = activeBody->substeps;

        if (m_SelectedShapeType == ShapeType::Sphere) {
            activeBody = std::make_unique<SoftBodySphere>(m_SphereConfig.radius, m_SphereConfig.stacks, m_SphereConfig.sectors);
        } else {
            activeBody = std::make_unique<SoftBodyCube>(m_CubeConfig.size, m_CubeConfig.resolution);
        }

        activeBody->k_1 = k1;
        activeBody->k_2 = k2;
        activeBody->k_3 = k3;
        activeBody->DAMPING = d1;
        activeBody->DAMPING_2 = d2;
        activeBody->DAMPING_3 = d3;
        activeBody->substeps = subs;
        activeBody->UpdateSpringParameters();
    }

    ImGui::Separator();

    ImGui::Text("Particles: %zu | Springs: %zu", activeBody->GetParticles().size(), activeBody->GetSprings().size());
    ImGui::Text("Volume: %.1f (Rest: %.1f)", activeBody->ComputeVolume(), activeBody->GetRestVolume());

    ImGui::Separator();

    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "Solver Config");
    ImGui::SliderInt("Substeps", &activeBody->substeps, 1, 40);

    ImGui::Separator();

    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "Spring Config");

    bool physicsChanged = false;

    if (ImGui::SliderFloat("Stiffness K1 (Shear)", &activeBody->k_1, 10.0f, 2000.0f, "%.0f")) { physicsChanged = true; }
    if (ImGui::SliderFloat("Stiffness K2 (Structural)", &activeBody->k_2, 10.0f, 2000.0f, "%.0f")) { physicsChanged = true; }
    if (ImGui::SliderFloat("Stiffness K3 (Bending)", &activeBody->k_3, 10.0f, 2000.0f, "%.0f")) { physicsChanged = true; }

    ImGui::Spacing();

    if (ImGui::SliderFloat("Damping 1 (Shear)", &activeBody->DAMPING, 0.0f, 150.0f, "%.1f")) { physicsChanged = true; }
    if (ImGui::SliderFloat("Damping 2 (Structural)", &activeBody->DAMPING_2, 0.0f, 150.0f, "%.1f")) { physicsChanged = true; }
    if (ImGui::SliderFloat("Damping 3 (Bending)", &activeBody->DAMPING_3, 0.0f, 150.0f, "%.1f")) { physicsChanged = true; }

    if (physicsChanged) {
        activeBody->UpdateSpringParameters();
    }

    ImGui::End();
}

void UI::EndFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UI::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}