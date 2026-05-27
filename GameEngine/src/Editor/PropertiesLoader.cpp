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

std::unordered_map<entt::id_type, PropertiesLoader::DrawFn> PropertiesLoader::componentFunc;
std::unordered_map<entt::id_type, std::string> PropertiesLoader::componentNames;

void PropertiesLoader::Reset()
{
    componentNames.clear();
    componentFunc.clear();

    RegisterComponent<TransformData>("Transform", [](Entity entity) {
            auto& t = entity.GetComponent<TransformData>();
            //ImGui::DragFloat3("Position", &t.position.x, 0.1f);
            //ImGui::DragFloat3("Rotation", &t.rotation.x, 0.1f);
            //ImGui::DragFloat3("Scale", &t.scale.x, 0.01f);
    });

    RegisterComponent<MetadataData>("Object", [](Entity entity) {
        auto& t = entity.GetComponent<MetadataData>();
        //ImGui::DragFloat3("Position", &t.position.x, 0.1f);
        //ImGui::DragFloat3("Rotation", &t.rotation.x, 0.1f);
        //ImGui::DragFloat3("Scale", &t.scale.x, 0.01f);
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