#include "EditorUI.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <functional>

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  include <commdlg.h>
#else
#  include <cstdio>
#endif

EditorUI::EditorUI(GLFWwindow* window) : window(window)
{
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding = 6.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;

    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;

    style.ItemSpacing = ImVec2(8, 5);
    style.FramePadding = ImVec2(6, 4);
    style.WindowPadding = ImVec2(10, 10);
    style.IndentSpacing = 16.0f;
    style.ScrollbarSize = 10.0f;

    ImVec4* c = style.Colors;

    c[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);
    c[ImGuiCol_ChildBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    c[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.12f, 0.13f, 0.96f);

    c[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    c[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
    c[ImGuiCol_TitleBgCollapsed] = ImVec4(0.08f, 0.08f, 0.09f, 0.70f);

    c[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    c[ImGuiCol_FrameBgActive] = ImVec4(0.24f, 0.24f, 0.27f, 1.00f);

    ImVec4 accent = ImVec4(0.40f, 0.65f, 1.00f, 1.00f);
    ImVec4 accentHover = ImVec4(0.50f, 0.73f, 1.00f, 1.00f);
    ImVec4 accentActive = ImVec4(0.30f, 0.55f, 0.90f, 1.00f);

    c[ImGuiCol_CheckMark] = accent;
    c[ImGuiCol_SliderGrab] = accent;
    c[ImGuiCol_SliderGrabActive] = accentActive;
    c[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.28f, 0.32f, 1.00f);
    c[ImGuiCol_ButtonActive] = accentActive;
    c[ImGuiCol_Header] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.22f, 0.22f, 0.26f, 1.00f);
    c[ImGuiCol_HeaderActive] = accent;

    c[ImGuiCol_Tab] = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
    c[ImGuiCol_TabHovered] = ImVec4(0.22f, 0.22f, 0.26f, 1.00f);
    c[ImGuiCol_TabActive] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);

    c[ImGuiCol_Separator] = ImVec4(0.22f, 0.22f, 0.25f, 1.00f);
    c[ImGuiCol_SeparatorHovered] = accentHover;
    c[ImGuiCol_SeparatorActive] = accent;

    c[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.26f, 0.30f, 0.50f);
    c[ImGuiCol_ResizeGripHovered] = accentHover;
    c[ImGuiCol_ResizeGripActive] = accent;

    c[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    c[ImGuiCol_ScrollbarGrab] = ImVec4(0.28f, 0.28f, 0.32f, 1.00f);
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.34f, 0.34f, 0.38f, 1.00f);
    c[ImGuiCol_ScrollbarGrabActive] = accent;

    ChangeSelectedID(AssetID(0));

    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init("#version 460");
}

EditorUI::~EditorUI() { Shutdown(); }

void EditorUI::Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void EditorUI::NewFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void EditorUI::RenderDockspace()
{
    static bool dockspaceOpen = true;
    static bool optFullscreen = true;

    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoDocking;

    if (optFullscreen)
    {
        const ImGuiViewport* viewport =
            ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        window_flags |=
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_MenuBar |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoBackground;
    }

    ImGui::Begin("DockSpace Window", &dockspaceOpen, window_flags);

    if (optFullscreen)
        ImGui::PopStyleVar(3);

    DrawMenuBar();

    ImGuiIO& io = ImGui::GetIO();

    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id =
            ImGui::GetID("MainDockSpace");

        ImGui::DockSpace(
            dockspace_id,
            ImVec2(0.0f, 0.0f),
            ImGuiDockNodeFlags_None
        );
    }

    ImGui::End();
}

void EditorUI::Render()
{
    int displayW, displayH;
    glfwGetFramebufferSize(window, &displayW, &displayH);

    const float PANEL_W = 240.0f;
    const float MENUBAR_H = 20.0f;
    const float STATUSBAR_H = 34.0f;
    const float panelTop = MENUBAR_H;
    const float panelHeight = (float)displayH - MENUBAR_H - STATUSBAR_H;

    RenderDockspace();
    

    //ImGui::SetNextWindowPos(ImVec2(0, panelTop));
    //ImGui::SetNextWindowSize(ImVec2(PANEL_W, panelHeight));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("Scene", nullptr);
    ImGui::PopStyleVar();
    DrawScenePanel();
    ImGui::End();

    //ImGui::SetNextWindowPos(ImVec2((float)displayW - PANEL_W, panelTop));
    //ImGui::SetNextWindowSize(ImVec2(PANEL_W, panelHeight));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("Properties", nullptr);
    ImGui::PopStyleVar();
    //DrawPropertiesPanel();
    ImGui::End();

    //ImGui::SetNextWindowPos(ImVec2(0, (float)displayH - STATUSBAR_H));
    //ImGui::SetNextWindowSize(ImVec2((float)displayW, STATUSBAR_H));
    //ImGui::SetNextWindowBgAlpha(0.90f);
    //ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    //ImGui::Begin("##StatusBar", nullptr,
    //    panelFlags |
    //    ImGuiWindowFlags_NoTitleBar |
    //    ImGuiWindowFlags_NoScrollbar |
    //    ImGuiWindowFlags_NoScrollWithMouse);
    //ImGui::PopStyleVar();
    ////DrawViewportStatusBar();
    //ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

//void EditorUI::RaytraceImageSave()
//{
//    raytracing->SaveImage(scene->raytrace_image_path);
//}
//
void EditorUI::DrawMenuBar()
{
    if (!ImGui::BeginMenuBar()) return;

    if (ImGui::BeginMenu("File"))
    {

        if (ImGui::MenuItem("New Scene"))
        {
            //onNewScene();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Open Scene"))
        {
            /*std::string path = OpenFileDialog("Scene files (*.json)", "*.json");
            if (!path.empty() && onLoadScene)
            {
                onLoadScene(path);

            }*/
        }

        if (ImGui::MenuItem("Save Scene"))
        {
           /* std::string path = SaveFileDialog("Scene files (*.json)", "*.json", "json");
            if (!path.empty() && onSaveScene)
                onSaveScene(path);*/
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Quit", "Alt+F4"))
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        ImGui::EndMenu();
    }

    ImGui::EndMenuBar();
}

void EditorUI::ChangeSelectedID(const AssetID& id)
{
    selectedModelIDReadOnly = id;
}

void EditorUI::DrawScenePanel()
{
    static int selectedNode = -1;

    ImGuiTreeNodeFlags base_flags = 
        ImGuiTreeNodeFlags_DrawLinesFull
        | ImGuiTreeNodeFlags_DefaultOpen
        | ImGuiTreeNodeFlags_OpenOnArrow   
        | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    auto NodeFlags = [&](int id) {
        return base_flags | (selectedNode == id ? ImGuiTreeNodeFlags_Selected : 0);
    };

    // --- Parent (id = 0) ---
    bool parentOpen = ImGui::TreeNodeEx("Parent##0", NodeFlags(0));
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        selectedNode = 0;

    if (parentOpen)
    {
        // --- Child 1 (id = 1) ---
        bool child1Open = ImGui::TreeNodeEx("Child 1##1", NodeFlags(1));
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            selectedNode = 1;

        if (child1Open)
        {
            // --- Child 1 > Child 1 (id = 2) ---
            bool child1_1Open = ImGui::TreeNodeEx("Child 1##2", NodeFlags(2));
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                selectedNode = 2;

            if (child1_1Open)
            {
                // --- Child 1 > Child 1 > Child 1 (id = 3) ---
                bool child1_1_1Open = ImGui::TreeNodeEx("Child 1##3", NodeFlags(3));
                if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                    selectedNode = 3;

                if (child1_1_1Open)
                    ImGui::TreePop();

                ImGui::TreePop();
            }
            ImGui::TreePop();
        }

        // --- Child 2 (id = 4) ---
        bool child2Open = ImGui::TreeNodeEx("Child 2##4", NodeFlags(4));
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            selectedNode = 4;

        if (child2Open)
            ImGui::TreePop();

        ImGui::TreePop();
    }
}
//{
//    float btnWidth = ImGui::GetContentRegionAvail().x;
//
//    ImGui::TextDisabled("Scene objects (%zu)", scene->models.size() + 1);
//    ImGui::Spacing();
//
//    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 2));
//
//    std::vector<std::tuple<std::string, AssetID, Model*>> sortedNames(scene->models.size() + 1);
//
//    uint32_t index = 0;
//    for (auto& [id, model] : scene->models)
//    {
//        sortedNames[index] = { model->name, id, model.get() };
//        index++;
//    }
//    sortedNames[index] = { "Camera", AssetID(1), nullptr };
//
//    std::sort(sortedNames.begin(), sortedNames.end(), [&](const std::tuple<std::string, AssetID, Model*>& a, const std::tuple<std::string, AssetID, Model*>& b) {
//        return std::get<0>(a) < std::get<0>(b);
//        });
//
//    for (auto& [name, id, model] : sortedNames)
//    {
//        ImGui::PushID(id.value);
//
//        char label[64];
//        std::snprintf(label, sizeof(label), "  %s", name.c_str());
//
//        bool selected = (selectedModelIDReadOnly.value == id.value);
//
//        if (selected) {
//            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.30f, 0.50f, 0.85f, 0.60f));
//        }
//
//        ImGuiSelectableFlags flags = ImGuiSelectableFlags_SpanAllColumns;
//
//        if (id == AssetID(1))
//        {
//            if (ImGui::Selectable(label, selected, flags, ImVec2(0, 18)))
//            {
//                ChangeSelectedID(AssetID(1));
//                cachedTranslation = scene->cameraPosition;
//                cachedRotation = scene->cameraRotation;
//                uiRayBounces = scene->GetRayBounces();
//                uiShadowBounces = scene->GetShadowBounces();
//                uiBatchSize = scene->GetBatchSize();
//                uiTargetSamples = scene->GetTargetSamples();
//                uiExposure = scene->GetExposure();
//                uiApplyGamma = scene->GetApplyGamma();
//                uiresolution = scene->GetResolution();
//                uifov = scene->GetFOV();
//            }
//
//            if (selected) ImGui::PopStyleColor();
//        }
//        else
//        {
//            if (ImGui::Selectable(label, selected, flags, ImVec2(0, 18))) {
//                ChangeSelectedID(id);
//                cachedTranslation = model->position;
//                cachedRotation = model->rotation;
//                cachedScale = model->scale;
//            }
//
//            if (selected) ImGui::PopStyleColor();
//
//            if (ImGui::BeginPopupContextItem())
//            {
//                if (ImGui::MenuItem("Delete")) {
//                    if (selectedModelIDReadOnly.value == id.value)
//                        ChangeSelectedID(AssetID(0));
//                    scene->ModelRemove(id);
//                    ImGui::EndPopup();
//                    ImGui::PopID();
//                    break;
//                }
//                if (ImGui::MenuItem("Duplicate")) {
//                    scene->ModelDuplicate(id);
//                }
//                ImGui::EndPopup();
//            }
//        }
//
//        ImGui::PopID();
//    }
//
//    ImGui::PopStyleVar();
//}
//
//void EditorUI::DrawPropertiesPanel()
//{
//    if (selectedModelIDReadOnly.value == 0 || (selectedModelIDReadOnly.value != 1 && scene->models.find(selectedModelIDReadOnly) == scene->models.end()))
//    {
//        ImGui::TextDisabled("No model selected.");
//        ImGui::Spacing();
//        ImGui::Separator();
//        DrawRenderSettingsSection();
//        return;
//    }
//
//    if (selectedModelIDReadOnly.value == 1)
//    {
//        char header[64];
//        std::snprintf(header, sizeof(header), "Camera");
//        ImGui::Text("%s", header);
//        ImGui::SameLine();
//        ImGui::TextDisabled("(ID %llu)", selectedModelIDReadOnly.value);
//        if (ImGui::IsItemHovered())
//            ImGui::SetTooltip("%llu", selectedModelIDReadOnly.value);
//        ImGui::Separator();
//        ImGui::Spacing();
//
//        DrawCameraTransformSection();
//        ImGui::Spacing();
//        DrawCameraSceneSection();
//        ImGui::Spacing();
//        DrawCameraSection();
//        ImGui::Spacing();
//    }
//    else
//    {
//
//        Model* model = scene->models[selectedModelIDReadOnly].get();
//
//        std::string name = model->name;
//
//        if (ImGui::InputText("##name", &name))
//        {
//            model->SetName(name);
//        }
//
//        ImGui::SameLine();
//        ImGui::TextDisabled("(ID %llu)", selectedModelIDReadOnly.value);
//        if (ImGui::IsItemHovered())
//            ImGui::SetTooltip("%llu", selectedModelIDReadOnly.value);
//        ImGui::Separator();
//        ImGui::Spacing();
//
//        DrawTransformSection(model);
//        ImGui::Spacing();
//        DrawMaterialSection(model);
//        ImGui::Spacing();
//    }
//
//    DrawRenderSettingsSection();
//}
//
//void EditorUI::DrawCameraTransformSection()
//{
//    if (!ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
//        return;
//
//    bool changed = false;
//
//    ImGui::PushItemWidth(-1);
//
//    ImGui::TextDisabled("Position");
//    changed |= ImGui::DragFloat3("##pos", glm::value_ptr(cachedTranslation),
//        0.01f, -1000.0f, 1000.0f, "%.3f");
//
//    ImGui::TextDisabled("Rotation");
//    changed |= ImGui::DragFloat3("##rot", glm::value_ptr(cachedRotation),
//        0.5f, -360.0f, 360.0f, "%.2f");
//
//    ImGui::PopItemWidth();
//
//    if (changed)
//    {
//        scene->cameraPosition = cachedTranslation;
//        scene->cameraRotation = cachedRotation;
//    }
//
//    ImGui::Spacing();
//
//    float w3 = (ImGui::GetContentRegionAvail().x - 8);
//    if (ImGui::Button("Copy editor transform", ImVec2(w3, 0)))
//    {
//        scene->cameraPosition = editor->cameraPosition;
//        scene->cameraRotation = editor->cameraRotation;
//        cachedTranslation = editor->cameraPosition;
//        cachedRotation = editor->cameraRotation;
//    }
//}
//
//void EditorUI::DrawCameraSceneSection()
//{
//    if (!ImGui::CollapsingHeader("Scene", ImGuiTreeNodeFlags_DefaultOpen))
//        return;
//
//    ImGui::PushItemWidth(-1);
//
//    ImGui::TextDisabled("Skybox");
//    DrawTextureSlot("Texture", [&](AssetID texture_id) {
//        scene->SkyboxSet(texture_id);
//        }, scene->GetSkyboxId(), false, "click to assign");
//}
//
//void EditorUI::DrawCameraSection()
//{
//    if (!ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
//        return;
//
//    ImGui::PushItemWidth(-1);
//
//    ImGui::TextDisabled("Resolution");
//    if (ImGui::DragInt2("##resolution", glm::value_ptr(uiresolution), 1, 1, 4096))
//    {
//        uiresolution.x = max(1, uiresolution.x);
//        uiresolution.y = max(1, uiresolution.y);
//        scene->SetResolution({ uiresolution.x, uiresolution.y });
//    }
//
//    ImGui::TextDisabled("FOV");
//    if (ImGui::SliderFloat("##fov", &uifov, 10.0f, 90.0f, "%.2f"))
//        scene->SetFOV(uifov);
//
//    ImGui::Spacing();
//
//    ImGui::TextDisabled("Ray Bounces");
//    if (ImGui::SliderInt("##raybounces", &uiRayBounces, 1, 32))
//        scene->SetRayBounces(uiRayBounces);
//
//    ImGui::TextDisabled("Shadow Bounces");
//    if (ImGui::SliderInt("##shadowbounces", &uiShadowBounces, 0, 32))
//        scene->SetShadowBounces(uiShadowBounces);
//
//    ImGui::TextDisabled("Batch Size");
//    if (ImGui::SliderInt("##batch", &uiBatchSize, 1, 256))
//        scene->SetBatchSize(uiBatchSize);
//
//    ImGui::TextDisabled("Target Samples");
//    if (ImGui::DragInt("##target", &uiTargetSamples, 1.0f, 1, INT_MAX, "%i"))
//    {
//        uiTargetSamples = max(uiTargetSamples, 1);
//        scene->SetTargetSamples(uiTargetSamples);
//    }
//
//    ImGui::Spacing();
//
//    ImGui::TextDisabled("Exposure");
//    if (ImGui::SliderFloat("##exposure", &uiExposure, 0.1f, 10.0f, "%.2f"))
//        scene->SetExposure(uiExposure);
//
//    ImGui::TextDisabled("Gamma Correction");
//    if (ImGui::Checkbox("##gamma", &uiApplyGamma))
//        scene->SetApplyGamma(uiApplyGamma);
//}
//
//
//void EditorUI::DrawTransformSection(Model* model)
//{
//    if (!ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
//        return;
//
//    bool changed = false;
//
//    ImGui::PushItemWidth(-1);
//
//    ImGui::TextDisabled("Position");
//    changed |= ImGui::DragFloat3("##pos", glm::value_ptr(cachedTranslation),
//        0.01f, -1000.0f, 1000.0f, "%.3f");
//
//    ImGui::TextDisabled("Rotation");
//    changed |= ImGui::DragFloat3("##rot", glm::value_ptr(cachedRotation),
//        0.5f, -360.0f, 360.0f, "%.2f");
//
//    ImGui::TextDisabled("Scale");
//    changed |= ImGui::DragFloat3("##scale", glm::value_ptr(cachedScale),
//        0.005f, 0.001f, 100.0f, "%.3f");
//
//    ImGui::PopItemWidth();
//
//    if (changed)
//    {
//        model->position = cachedTranslation;
//        model->rotation = cachedRotation;
//        model->scale = cachedScale;
//        model->RecalculateMatrix();
//    }
//
//    ImGui::Spacing();
//
//    float w3 = (ImGui::GetContentRegionAvail().x - 8) / 3.0f;
//    if (ImGui::Button("Reset Pos", ImVec2(w3, 0)))
//    {
//        model->ChangePosition(glm::vec3(0.0f));
//        cachedTranslation = glm::vec3(0.0f);
//    }
//    ImGui::SameLine();
//    if (ImGui::Button("Reset Rot", ImVec2(w3, 0)))
//    {
//        model->ChangeRotation(glm::vec3(0.0f));
//        cachedRotation = glm::vec3(0.0f);
//    }
//    ImGui::SameLine();
//    if (ImGui::Button("Reset Scl", ImVec2(w3, 0)))
//    {
//        model->ChangeScale(glm::vec3(1.0f));
//        cachedScale = glm::vec3(1.0f);
//    }
//}
//
//void EditorUI::DrawMaterialSection(Model* model)
//{
//    if (!ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
//        return;
//
//    ImGui::PushItemWidth(-1);
//
//    ImGui::TextDisabled("Albedo");
//    float albedo[3] = { model->albedo.r, model->albedo.g, model->albedo.b };
//    if (ImGui::ColorEdit3("##albedo", albedo,
//        ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_PickerHueBar))
//        model->albedo = glm::vec3(albedo[0], albedo[1], albedo[2]);
//
//    ImGui::TextDisabled("Emissive");
//    float emissive[3] = { model->emissive.r, model->emissive.g, model->emissive.b };
//    if (ImGui::ColorEdit3("##emissive", emissive,
//        ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_HDR))
//        model->emissive = glm::vec3(emissive[0], emissive[1], emissive[2]);
//
//    ImGui::Spacing();
//
//    int material_type = model->materialType;
//    const char* items[] = {
//        "Metallic / Roughness",
//        "Specular / Glossiness",
//    };
//    ImGui::TextDisabled("Material Type");
//    if (ImGui::Combo(" ", &material_type, items, IM_ARRAYSIZE(items)))
//    {
//        model->materialType = (uint32_t)material_type;
//    }
//
//    if (material_type == 0)
//    {
//        ImGui::TextDisabled("Metallic");
//        ImGui::SliderFloat("##metallic", &model->metallic, 0.0f, 1.0f, "%.2f");
//        ImGui::TextDisabled("Roughness");
//        ImGui::SliderFloat("##roughness", &model->roughness, 0.0f, 1.0f, "%.2f");
//    }
//    else if (material_type == 1)
//    {
//        ImGui::TextDisabled("Specular");
//        ImGui::SliderFloat("##specular", &model->roughness, 0.0f, 1.0f, "%.2f");
//        ImGui::TextDisabled("Glossiness");
//        ImGui::SliderFloat("##glossiness", &model->metallic, 0.0f, 1.0f, "%.2f");
//    }
//
//    ImGui::TextDisabled("Transmission");
//    ImGui::SliderFloat("##transmit", &model->transmission, 0.0f, 1.0f, "%.2f");
//    ImGui::TextDisabled("IOR");
//    ImGui::SliderFloat("##ior", &model->ior, 1.0f, 3.0f, "%.3f");
//
//    ImGui::PopItemWidth();
//
//    ImGui::Spacing();
//    ImGui::TextDisabled("Textures");
//
//    DrawTextureSlot("Albedo Map", [&](AssetID texture_id) {
//        scene->ModelBindColorTexture(model->id, texture_id);
//        }, model->GetColorTexture(), true, "click to assign");
//
//    DrawTextureSlot("Normal Map", [&](AssetID texture_id) {
//        scene->ModelBindNormalTexture(model->id, texture_id);
//        }, model->GetNormalTexture(), false, "click to assign");
//
//    if (material_type == 0)
//    {
//        DrawTextureSlot("Metal/Rough", [&](AssetID texture_id) {
//            scene->ModelBindMetallicRoughnessTexture(model->id, texture_id);
//            }, model->GetMetallicRoughnessTexture(), false, "click to assign");
//    }
//    else if (material_type == 1)
//    {
//        DrawTextureSlot("Specular/Gloss", [&](AssetID texture_id) {
//            scene->ModelBindMetallicRoughnessTexture(model->id, texture_id);
//            }, model->GetMetallicRoughnessTexture(), false, "click to assign");
//    }
//
//    DrawTextureSlot("Emissive Map", [&](AssetID texture_id) {
//        scene->ModelBindEmissiveTexture(model->id, texture_id);
//        }, model->GetEmissiveTexture(), true, "click to assign");
//
//    ImGui::Spacing();
//    ImGui::TextDisabled("Mesh");
//    ModelAsset* mesh = scene->GetMesh(model->id_mesh);
//    if (mesh)
//    {
//        ImGui::Text("  Vertices  : %zu", mesh->vertices.size());
//        ImGui::Text("  Triangles : %zu", mesh->indices.size() / 3);
//    }
//    else
//    {
//        ImGui::TextDisabled("  (no mesh assigned)");
//    }
//}
//
//void EditorUI::DrawTextureSlot(const char* label, std::function<void(AssetID)> texture_replace, AssetID texID, bool needSRGB, const char* hint)
//{
//    ImGui::PushID(label);
//
//    bool hasTexture = (texID.value != 0) && (scene->GetTexture(texID) != nullptr);
//
//    ImVec2 previewSize(28, 28);
//
//    if (hasTexture)
//    {
//        GLuint glTexID = scene->GetTexture(texID)->id_texture;
//        ImGui::Image(
//            (ImTextureID)(intptr_t)glTexID,
//            previewSize,
//            ImVec2(0, 1), ImVec2(1, 0)
//        );
//    }
//    else
//    {
//        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.18f, 0.20f, 1.0f));
//        ImGui::Button("##empty", previewSize);
//        ImGui::PopStyleColor();
//    }
//
//    if (ImGui::IsItemClicked())
//    {
//        std::string path = OpenFileDialog(
//            "Image files (*.png *.jpg *.hdr *.exr)",
//            "*.png;*.jpg;*.jpeg;*.hdr;*.exr");
//        if (!path.empty() && importTexture)
//            texture_replace(importTexture(path, needSRGB));
//    }
//
//    ImGui::SameLine();
//    ImGui::BeginGroup();
//    ImGui::TextUnformatted(label);
//    if (hasTexture)
//        ImGui::TextDisabled("ID %u", texID.value);
//    else
//        ImGui::TextDisabled("%s", hint);
//    ImGui::EndGroup();
//
//    if (ImGui::BeginPopupContextItem("##texctx"))
//    {
//        if (ImGui::MenuItem("Clear texture")) texture_replace(AssetID(0));
//        ImGui::EndPopup();
//    }
//
//    ImGui::PopID();
//    ImGui::Spacing();
//}
//
//void EditorUI::DrawRenderSettingsSection()
//{
//    if (!ImGui::CollapsingHeader("Render Settings", ImGuiTreeNodeFlags_DefaultOpen))
//        return;
//
//    ImGui::PushItemWidth(-1);
//
//    bool rt = raytracing->IsRaytracing();
//    ImGui::TextDisabled("Raytracing");
//    if (ImGui::Checkbox("##rt_enabled", &rt))
//    {
//        if (rt) editor->RaytraceStart();
//        else    editor->RaytraceStop();
//    }
//    ImGui::SameLine();
//    ImGui::TextUnformatted(rt ? "Active" : "Inactive");
//
//    if (rt)
//    {
//        ImGui::Spacing();
//        float progress = (scene->GetTargetSamples() > 0)
//            ? glm::clamp((float)editor->GetFrameIndex() / (float)scene->GetTargetSamples(),
//                0.0f, 1.0f)
//            : 0.0f;
//        char progressLabel[64];
//        std::snprintf(progressLabel, sizeof(progressLabel),
//            "Frame %u / %d", editor->GetFrameIndex(), scene->GetTargetSamples());
//        ImGui::ProgressBar(progress, ImVec2(-1, 0), progressLabel);
//    }
//
//    ImGui::Spacing();
//    ImGui::Separator();
//    ImGui::Spacing();
//
//    float bw = ImGui::GetContentRegionAvail().x;
//
//    if (ImGui::Button("Change image path", ImVec2(bw, 0)))
//    {
//        std::string path = SaveFileDialog("PNG image (*.png)", "*.png", "png");
//        if (!path.empty())
//            scene->raytrace_image_path = path;
//    }
//
//    ImGui::Spacing();
//
//    ImGui::TextDisabled("Path: %s", scene->raytrace_image_path.c_str());
//
//    if (ImGui::IsItemHovered())
//        ImGui::SetTooltip("%s", scene->raytrace_image_path.c_str());
//
//    ImGui::Spacing();
//
//    bool canSave = editor->GetFrameIndex() > 0;
//    if (!canSave) ImGui::PushStyleVar(ImGuiStyleVar_Alpha,
//        ImGui::GetStyle().Alpha * 0.4f);
//
//
//    if (ImGui::Button("Save Image", ImVec2(bw, 0)) && canSave)
//    {
//        RaytraceImageSave();
//    }
//
//    if (!canSave)
//    {
//        ImGui::PopStyleVar();
//        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
//            ImGui::SetTooltip("Render at least one frame first.");
//    }
//
//    ImGui::PopItemWidth();
//
//    ImGui::Spacing();
//    ImGui::Separator();
//    ImGui::Spacing();
//    ImGui::TextDisabled("Scene camera");
//    glm::vec3 cp = editor->cameraPosition;
//    glm::vec3 cr = editor->cameraRotation;
//    ImGui::Text("  Pos  %.2f  %.2f  %.2f", cp.x, cp.y, cp.z);
//    ImGui::Text("  Rot  %.1f  %.1f  %.1f", cr.x, cr.y, cr.z);
//
//    if (ImGui::Button("Reset Camera", ImVec2(-1, 0)))
//    {
//        editor->cameraPosition = glm::vec3(0, 1, -10);
//        editor->cameraRotation = glm::vec3(0, 0, 0);
//        editor->RecalculateViewMatrix();
//    }
//}
//
//void EditorUI::DrawViewportStatusBar()
//{
//    int w, h;
//    glfwGetFramebufferSize(window, &w, &h);
//
//    bool rt = raytracing->IsRaytracing();
//    if (rt)
//    {
//        ImGui::PushStyleColor(ImGuiCol_Button,
//            ImVec4(0.25f, 0.50f, 0.90f, 1.0f));
//        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
//            ImVec4(0.30f, 0.58f, 1.00f, 1.0f));
//    }
//    if (ImGui::Button(rt ? "  Stop Raytracing [F]  "
//        : "  Start Raytracing [F]  ")) {
//        if (!raytracing->IsRaytracing()) editor->RaytraceStart();
//        else editor->RaytraceStop();
//    }
//    if (rt) ImGui::PopStyleColor(2);
//
//    ImGui::SameLine(0, 16);
//
//    if (rt)
//    {
//        float progress = (scene->GetTargetSamples() > 0)
//            ? glm::clamp((float)editor->GetFrameIndex() / (float)scene->GetTargetSamples(),
//                0.0f, 1.0f)
//            : 0.0f;
//        char progressLabel[64];
//        std::snprintf(progressLabel, sizeof(progressLabel),
//            "Frame %u / %d", editor->GetFrameIndex(), scene->GetTargetSamples());
//        ImGui::ProgressBar(progress, ImVec2(256, 0), progressLabel);
//        ImGui::SameLine(0, 16);
//    }
//
//    ImGui::TextDisabled("RMB drag = rotate   WASD/QE = move   Shift = fast");
//
//    if (editor->GetFrameIndex() > 0)
//    {
//        float btnW = 150.0f;
//        float rightX = (float)w - btnW - ImGui::GetStyle().WindowPadding.x;
//        if (rightX > ImGui::GetCursorPosX() + 16.0f)
//        {
//            ImGui::SameLine(rightX);
//            ImGui::PushStyleColor(ImGuiCol_Button,
//                ImVec4(0.18f, 0.38f, 0.18f, 1.0f));
//            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
//                ImVec4(0.22f, 0.46f, 0.22f, 1.0f));
//            if (ImGui::Button("Save Image", ImVec2(btnW, 0)))
//            {
//                RaytraceImageSave();
//            }
//            ImGui::PopStyleColor(2);
//        }
//    }
//}
//
//std::string EditorUI::OpenFileDialog(const char* /*filterDesc*/,
//    const char* /*filterPattern*/)
//{
//#ifdef _WIN32
//    char filename[MAX_PATH] = {};
//    OPENFILENAMEA ofn = {};
//    ofn.lStructSize = sizeof(ofn);
//    ofn.hwndOwner = nullptr;
//    ofn.lpstrFile = filename;
//    ofn.nMaxFile = MAX_PATH;
//    ofn.lpstrFilter = "All Files\0*.*\0";
//    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
//    if (GetOpenFileNameA(&ofn)) return std::string(filename);
//    return {};
//#else
//    FILE* pipe = popen(
//        "zenity --file-selection 2>/dev/null || "
//        "kdialog --getopenfilename 2>/dev/null", "r");
//    if (!pipe) return {};
//    char buf[4096] = {};
//    if (fgets(buf, sizeof(buf), pipe)) {
//        pclose(pipe);
//        size_t len = strlen(buf);
//        while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
//            buf[--len] = '\0';
//        return std::string(buf);
//    }
//    pclose(pipe);
//    return {};
//#endif
//}
//
//std::string EditorUI::SaveFileDialog(const char* /*filterDesc*/,
//    const char* /*filterPattern*/,
//    const char* defaultExt)
//{
//#ifdef _WIN32
//    char filename[MAX_PATH] = {};
//    OPENFILENAMEA ofn = {};
//    ofn.lStructSize = sizeof(ofn);
//    ofn.hwndOwner = nullptr;
//    ofn.lpstrFile = filename;
//    ofn.nMaxFile = MAX_PATH;
//    ofn.lpstrFilter = "All Files\0*.*\0";
//    ofn.lpstrDefExt = defaultExt;
//    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
//    if (GetSaveFileNameA(&ofn)) return std::string(filename);
//    return {};
//#else
//    char cmd[256];
//    std::snprintf(cmd, sizeof(cmd),
//        "zenity --file-selection --save --confirm-overwrite 2>/dev/null || "
//        "kdialog --getsavefilename 2>/dev/null");
//    FILE* pipe = popen(cmd, "r");
//    if (!pipe) return {};
//    char buf[4096] = {};
//    if (fgets(buf, sizeof(buf), pipe)) {
//        pclose(pipe);
//        size_t len = strlen(buf);
//        while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
//            buf[--len] = '\0';
//
//        std::string result(buf);
//        if (defaultExt && result.find('.') == std::string::npos)
//            result += std::string(".") + defaultExt;
//        return result;
//    }
//    pclose(pipe);
//    return {};
//#endif
//}