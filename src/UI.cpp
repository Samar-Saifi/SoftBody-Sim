#include "UI.h"
#include "Sphere.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


void UI::Initialize(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void UI::BeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UI::Draw(Sphere*& globalSphere)
{
    ImGui::Begin("Shape Spawner Control Panel");

    if (globalSphere == nullptr) {
        ImGui::Text("No active simulation target found.");
        ImGui::End();
        return;
    }

    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Sphere Geometry Config");

    static float uiRadius = 5.0f;
    static int uiStacks = 15;
    static int uiSectors = 15;

    ImGui::SliderFloat("Sphere Radius", &uiRadius, 4, 20);
    ImGui::SliderInt("Mesh Stacks", &uiStacks, 4, 20);
    ImGui::SliderInt("Mesh Sectors", &uiSectors, 4, 20);

    ImGui::Spacing();

    if (ImGui::Button("Spawn / Regenerate Sphere", ImVec2(-1, 30)))
    {
        globalSphere->Rebuild(uiRadius, uiStacks, uiSectors);
        globalSphere->UpdateSpringParameters();
    }

    ImGui::Separator();


    if (ImGui::SliderInt("substeps", &globalSphere->substeps, 1, 40, "%d"))

    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "Spring Config");

    bool physicsChanged = false;

    if (ImGui::SliderFloat("Stiffness K1 (Shear)", &globalSphere->k_1, 10.0f, 2000.0f, "%.0f")) { physicsChanged = true; }
    if (ImGui::SliderFloat("Stiffness K2 (Structural)", &globalSphere->k_2, 10.0f, 2000.0f, "%.0f")) { physicsChanged = true; }
    if (ImGui::SliderFloat("Stiffness K3 (Bending)", &globalSphere->k_3, 10.0f, 2000.0f, "%.0f")) { physicsChanged = true; }

    ImGui::Spacing();

    if (ImGui::SliderFloat("Damping 1 (Shear)", &globalSphere->DAMPING, 0.0f, 150.0f, "%.1f")) { physicsChanged = true; }
    if (ImGui::SliderFloat("Damping 2 (Structural)", &globalSphere->DAMPING_2, 0.0f, 150.0f, "%.1f")) { physicsChanged = true; }
    if (ImGui::SliderFloat("Damping 3 (Bending)", &globalSphere->DAMPING_3, 0.0f, 150.0f, "%.1f")) { physicsChanged = true; }

    if (physicsChanged)
    {
        globalSphere->UpdateSpringParameters();
    }

    ImGui::End();
}

void UI::EndFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UI::Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}