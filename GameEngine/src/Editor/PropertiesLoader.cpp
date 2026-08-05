#include "PropertiesLoader.h"

#include "Components/ComponentTransform.h"
#include "Components/ComponentMetadata.h"
#include "Components\ComponentScript.h"
#include "Components\ComponentCamera.h"
#include "Scene.h"
#include "Scripting\ScriptSystem.h"
#include "Scripting\ScriptRegistry.h"
#include "Components/ComponentSpriteRenderer.h"
#include "Core/AssetDatabase.h"
#include "Assets/TextureAsset.h"
#include "SimpleAPI\Renderer.h"
#include "SimpleAPI/SpriteRenderer.h"
#include "SimpleAPI/Camera.h"

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
std::unordered_map<entt::id_type, PropertiesLoader::AddFn> PropertiesLoader::componentAddFunc;
std::unordered_map<entt::id_type, PropertiesLoader::ComponentData> PropertiesLoader::componentData;
std::map<std::string, std::vector<entt::id_type>> PropertiesLoader::addFolderList;

void PropertiesLoader::Reset()
{
    componentData.clear();
    componentFunc.clear();
    addFolderList.clear();

    RegisterComponent<Vortx::TransformData>("Transform", "Meta", true, [](Entity entity) {
        //auto& t = entity.GetComponent<Vortx::TransformData>();
        Transform t = Transform(entity);
        ImGui::Spacing();

        glm::vec2 pos = t.LocalPosition();
        float rot = t.LocalRotation();
        glm::vec2 sca = t.LocalScale();

        ImGui::TextDisabled("Position");
        if (ImGui::DragFloat2("##pos", (float*)&pos, 0.1f, 0.0f, 0.0f, "%.2f", 0))
        {
            t.SetLocalPosition(pos);
        }

        ImGui::TextDisabled("Rotation");
        if (ImGui::DragFloat("##rot", (float*)&rot, 0.1f, 0.0f, 0.0f, "%.2f", 0))
        {
            t.SetLocalRotation(rot);
        }

        ImGui::TextDisabled("Scale");
        if (ImGui::DragFloat2("##sca", (float*)&sca, 0.1f, 0.0f, 0.0f, "%.2f", 0))
        {
            t.SetLocalScale(sca);
        }

        ImGui::Spacing();
    }, [](Entity entity) {
        GameObject(entity).AddComponent<Transform>();
    });

    RegisterComponent<MetadataData>("Object", "Meta", true, [](Entity entity) {
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
    }, [](Entity entity) {
        entity.AddComponent<Vortx::MetadataData>();
    });

    RegisterComponent<ScriptComponent>("Scripts", "Meta", false, [](Entity entity)
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
    }, [](Entity entity) {
        entity.AddComponent<Vortx::ScriptComponent>();
    });

    RegisterComponent<SpriteRendererData>("Sprite Renderer", "Rendering", true, [](Entity entity) {
        auto& sr = entity.GetComponent<SpriteRendererData>();

        ImGui::Spacing();
        ImGui::TextDisabled("Texture");
        ImGui::SameLine();

        Vortx::TextureAsset* tex = Vortx::AssetDatabase::GetLoaded<Vortx::TextureAsset>(sr.textureID);

        if (tex)
            ImGui::ImageButton("##texPreview", (ImTextureID)(intptr_t)Vortx::Renderer::GetNativeTextureHandle(tex->handle), ImVec2(48, 48));
        else
            ImGui::Button("None", ImVec2(48, 48));

        if (ImGui::BeginDragDropTarget())
        {
            if (auto* payload = ImGui::AcceptDragDropPayload("ASSET_TEXTURE"))
            {
                SpriteRenderer(entity).SetTexture(*(Vortx::AssetID*)payload->Data);
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

        ImGui::TextDisabled("Layer");
        ImGui::DragInt("##lay", &sr.layer);
        ImGui::Spacing();

        ImGui::TextDisabled("Order In Layer");
        ImGui::DragInt("##orderInLay", &sr.orderInLayer);
        ImGui::Spacing();
    }, [](Entity entity) {
        GameObject(entity).AddComponent<SpriteRenderer>();
    });

    RegisterComponent<Vortx::CameraData>("Camera", "Logic", true, [](Entity entity) {
        //auto& t = entity.GetComponent<Vortx::CameraData>();
        Camera t = Camera(entity);

        float size = t.GetSize();

        ImGui::Spacing();

        ImGui::TextDisabled("Size");
        if (ImGui::DragFloat("##siz", &size, 0.1f, 0.0f, 0.0f, "%.2f", 0))
        {
            t.SetSize(size);
        }

        ImGui::Spacing();
    }, [](Entity entity) {
        GameObject(entity).AddComponent<Camera>();
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

void PropertiesLoader::AddComponent(Entity entity, entt::id_type hash)
{
    auto it = componentAddFunc.find(hash);

    if (it != componentAddFunc.end())
    {
        it->second(entity);
    }
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
        for (auto& i : addFolderList)
        {
            bool headerAdded = 0;
            bool headerOpen = 0;

            if (i.first == "Meta") continue;

            for (auto& j : i.second)
            {

                if (entity.HasComponent(j)) continue;

                std::string lower = componentData[j].name;
                std::transform(lower.begin(), lower.end(), lower.begin(),
                    [](unsigned char c) { return (char)std::tolower(c); });

                if (!search.empty() && lower.find(search) == std::string::npos)
                    continue;

                if (!headerAdded)
                {
                    headerAdded = 1;
                    headerOpen = ImGui::TreeNodeEx(i.first.c_str());
                }

                any = true;

                if (headerOpen) {
                    if (ImGui::Selectable(componentData[j].name.c_str()))
                    {
                        AddComponent(entity, j);
                        //entity.AddComponent(j);
                        searchBuf[0] = '\0';
                        ImGui::CloseCurrentPopup();
                    }
                }
            }

            if (headerOpen)
            {
                ImGui::TreePop();
            }
        }

        bool scriptsHeaderAdded = 0;
        bool scriptsheaderOpen = 0;

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
                scriptsheaderOpen = ImGui::TreeNodeEx("Scripts");
            }

            any = true;

            if (scriptsheaderOpen) {
                if (ImGui::Selectable(display.c_str()))
                {
                    ScriptSystem::Instance().AddScript(entity, key);
                    searchBuf[0] = '\0';
                    ImGui::CloseCurrentPopup();
                }
            }
        }

        if (scriptsheaderOpen)
        {
            ImGui::TreePop();
        }

        if (!any)
            ImGui::TextDisabled("No components found");
    }
    ImGui::EndChild();

    ImGui::EndPopup();
}