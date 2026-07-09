#include "PropertiesLoader.h"

#include "Components/ComponentTransform.h"
#include "Components/ComponentMetadata.h"
#include "Components\ComponentScript.h"
#include "Scene.h"
#include "Scripting\ScriptSystem.h"
#include "Scripting\ScriptRegistry.h"
#include "Components/ComponentSpriteRenderer.h"
#include "Core/AssetDatabase.h"
#include "Assets/TextureAsset.h"

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
std::unordered_map<entt::id_type, PropertiesLoader::ComponentData> PropertiesLoader::componentData;

void PropertiesLoader::Reset()
{
    componentData.clear();
    componentFunc.clear();

    RegisterComponent<Vortx::TransformData>("Transform", true, [](Entity entity) {
        auto& t = entity.GetComponent<Vortx::TransformData>();

        ImGui::Spacing();

        ImGui::TextDisabled("Position");
        ImGui::DragFloat2("##pos", (float*)&t.position, 0.1f, 0.0f, 0.0f, "%.2f", 0);
        ImGui::TextDisabled("Rotation");
        ImGui::DragFloat("##rot", (float*)&t.rotation, 0.1f, 0.0f, 0.0f, "%.2f", 0);
        ImGui::TextDisabled("Scale");
        ImGui::DragFloat2("##sca", (float*)&t.scale, 0.1f, 0.0f, 0.0f, "%.2f", 0);

        ImGui::Spacing();
    });

    RegisterComponent<MetadataData>("Object", true, [](Entity entity) {
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

    RegisterComponent<ScriptComponent>("Scripts", false, [](Entity entity)
    {
        auto& sc = entity.GetComponent<ScriptComponent>();

        //ImGui::Spacing();

        for (int i = 0; i < (int)sc.scripts.size(); i++)
        {
            /*if (i != 0) 
            {
                ImGui::Spacing();
            }*/

            ImGui::PushID(i);

            if (ImGui::CollapsingHeader(PropertiesLoader::CleanScriptName(sc.scripts[i].name).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
            {

                if (ImGui::SmallButton("x"))
                {
                    ScriptSystem::Instance().RemoveScript(entity, sc.scripts[i].name);
                    ImGui::PopID();
                    break;
                }

            }

            ImGui::PopID();
        }

        if (sc.scripts.empty())
            ImGui::TextDisabled("No scripts attached");

       // ImGui::Spacing();
    });

    RegisterComponent<SpriteRendererData>("Sprite Renderer", true, [](Entity entity) {
        auto& sr = entity.GetComponent<SpriteRendererData>();

        ImGui::Spacing();
        ImGui::TextDisabled("Texture");
        ImGui::SameLine();

        Vortx::TextureAsset* tex = Vortx::AssetDatabase::GetLoaded<Vortx::TextureAsset>(sr.textureID);

        if (tex)
            ImGui::ImageButton("##texPreview", (ImTextureID)(intptr_t)tex->glHandle, ImVec2(48, 48));
        else
            ImGui::Button("None", ImVec2(48, 48));

        if (ImGui::BeginDragDropTarget())
        {
            if (auto* payload = ImGui::AcceptDragDropPayload("ASSET_TEXTURE"))
            {
                sr.textureID = *(Vortx::AssetID*)payload->Data;
                if (Vortx::Scene::Current())
                    Vortx::Scene::Current()->SyncAssets();
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::Spacing();
        ImGui::TextDisabled("Tint");
        ImGui::ColorEdit4("##tint", (float*)&sr.color);

        ImGui::Checkbox("Flip X", &sr.flipX);
        ImGui::SameLine();
        ImGui::Checkbox("Flip Y", &sr.flipY);
        ImGui::Spacing();
    });
}

std::string PropertiesLoader::CleanScriptName(const std::string& raw)
{
    std::string name = raw;

    // Strip MSVC typeid prefixes: "class Foo" / "struct Foo"
    const char* prefixes[] = { "class ", "struct " };
    for (const char* p : prefixes)
    {
        size_t len = strlen(p);
        if (name.compare(0, len, p) == 0)
        {
            name = name.substr(len);
            break;
        }
    }

    // Strip namespace qualifiers — show only the last segment
    size_t pos = name.find_last_of(':');
    if (pos != std::string::npos)
        name = name.substr(pos + 1);

    return name;
}

void PropertiesLoader::DrawPropertiesPanel(const AssetID& selected)
{
    if (Scene::Current() == nullptr) return;

    auto it = Scene::Current()->gameObjectsRegister.find(selected);
    if (it == Scene::Current()->gameObjectsRegister.end()) return;

    Entity entity = it->second.GetEntity();

    for (auto&& [id, storage] : entity.GetRegistry().Storage())
    {
        if (!storage.contains(entity.GetValue())) continue;

        auto cit = componentFunc.find(id);
        if (cit == componentFunc.end()) continue;

        ComponentData& data = componentData[id];

        if (data.useHeader)
        {
            if (ImGui::CollapsingHeader(data.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                cit->second(entity);
        }
        else
        {
            cit->second(entity);
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    float buttonWidth = 200.0f;
    float avail = ImGui::GetContentRegionAvail().x;
    ImGui::SetCursorPosX(std::max(0.0f, (avail - buttonWidth) * 0.5f));

    if (ImGui::Button("Add Component", ImVec2(buttonWidth, 0)))
        ImGui::OpenPopup("AddComponentPopup");

    DrawAddComponentPopup(entity);
}

void PropertiesLoader::DrawAddComponentPopup(Entity entity)
{
    if (!ImGui::BeginPopup("AddComponentPopup"))
        return;

    static char searchBuf[128] = "";

    if (ImGui::IsWindowAppearing())
        ImGui::SetKeyboardFocusHere();

    ImGui::SetNextItemWidth(220.0f);
    ImGui::InputTextWithHint("##search", "Search...", searchBuf, sizeof(searchBuf));
    ImGui::Separator();

    std::string search = searchBuf;
    std::transform(search.begin(), search.end(), search.begin(),
        [](unsigned char c) { return (char)std::tolower(c); });

    auto& scripts = ScriptRegistry::Instance().GetAll();
    bool any = false;

    if (ImGui::BeginChild("##componentList", ImVec2(220.0f, 180.0f)))
    {
        bool scriptsHeaderAdded = 0;
        bool headerOpen = 0;

        for (auto& [key, info] : scripts)
        {
            std::string display = CleanScriptName(key);

            std::string lower = display;
            std::transform(lower.begin(), lower.end(), lower.begin(),
                [](unsigned char c) { return (char)std::tolower(c); });

            if (!search.empty() && lower.find(search) == std::string::npos)
                continue;

            if (!scriptsHeaderAdded)
            {
                scriptsHeaderAdded = 1;
                headerOpen = ImGui::TreeNodeEx("Scripts");
            }

            any = true;

            if (headerOpen) {
                if (ImGui::Selectable(display.c_str()))
                {
                    ScriptSystem::Instance().AddScript(entity, key);
                    searchBuf[0] = '\0';
                    ImGui::CloseCurrentPopup();
                }
            }
        }

        if (headerOpen)
        {
            ImGui::TreePop();
        }

        if (!any)
            ImGui::TextDisabled("No components found");
    }
    ImGui::EndChild();

    ImGui::EndPopup();
}