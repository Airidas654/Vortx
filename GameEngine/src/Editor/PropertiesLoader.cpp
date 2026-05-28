#include "PropertiesLoader.h"

#include "Components/ComponentTransform.h"
#include "Components/ComponentMetadata.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"

#include <iostream>

std::unordered_map<entt::id_type, PropertiesLoader::DrawFn> PropertiesLoader::componentFunc;
std::unordered_map<entt::id_type, std::string> PropertiesLoader::componentNames;

void PropertiesLoader::Reset()
{
    componentNames.clear();
    componentFunc.clear();

    RegisterComponent<TransformData>("Transform", [](Entity entity) {
            auto& t = entity.GetComponent<TransformData>();

            ImGui::Spacing();

            ImGui::TextDisabled("Position");
            ImGui::DragFloat2("##pos", (float*)&t.position, 0.1f, 0.0f, 0.0f, "%.2f", 0);
            ImGui::TextDisabled("Rotation");
            ImGui::DragFloat("##rot", (float*)&t.rotation, 0.1f, 0.0f, 0.0f, "%.2f", 0);
            ImGui::TextDisabled("Scale");
            ImGui::DragFloat2("##sca", (float*)&t.scale, 0.1f, 0.0f, 0.0f, "%.2f", 0);

            ImGui::Spacing();
    });

    RegisterComponent<MetadataData>("Object", [](Entity entity) {
        auto& t = entity.GetComponent<MetadataData>();
        
        ImGui::Spacing();
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("Name");
        ImGui::SameLine();
        ImGui::InputText("##name", &t.name, ImGuiInputTextFlags_AutoSelectAll); 

        ImGui::Spacing();
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("Tag");
        ImGui::SameLine();
        ImGui::InputText("##tag", &t.tag, ImGuiInputTextFlags_AutoSelectAll);

        ImGui::Spacing();
    });
}

void PropertiesLoader::DrawPropertiesPanel(std::optional<Entity>& selected)
{
    if (!selected.has_value()) {
        return;
    }

    Entity entity = selected.value();

    for (auto&& [id, storage] : entity.GetRegistry().Storage())
    {
        if (!storage.contains(entity.GetValue())) continue;

        auto it = componentFunc.find(id);
        if (it == componentFunc.end()) continue;

        std::string& label = componentNames[id];
        if (ImGui::CollapsingHeader(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            it->second(entity);
        }
    }
}