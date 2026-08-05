#include "AssetPropertiesLoader.h"

#include "Core/AssetDatabase.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"

std::unordered_map<AssetType, AssetPropertiesLoader::AssetData> AssetPropertiesLoader::assetData;
std::unordered_map<Vortx::AssetType, AssetPropertiesLoader::DrawFn> AssetPropertiesLoader::assetFunc;

void AssetPropertiesLoader::Reset()
{
    assetFunc.clear();
    assetData.clear();

    RegisterComponent(AssetType::Texture, true, [](const Vortx::AssetID& id, AssetMeta& meta) {

        bool filterLinear = meta.importSettings.value("filterLinear", true);
        bool wrapRepeat = meta.importSettings.value("wrapRepeat", false);
        
        bool changed = false;
        changed |= ImGui::Checkbox("Linear Filtering", &filterLinear);
        changed |= ImGui::Checkbox("Repeat Wrap", &wrapRepeat);

        if (changed)
        {
            nlohmann::json settings = meta.importSettings;
            settings["filterLinear"] = filterLinear;
            settings["wrapRepeat"] = wrapRepeat;
            Vortx::AssetDatabase::SetImportSettings(id, settings);
        }

    });
}

void AssetPropertiesLoader::DrawPropertiesPanel(const Vortx::AssetID& id, const std::string& path)
{
    Vortx::AssetMeta meta;
    if (!Vortx::AssetDatabase::TryGetMeta(id, meta))
    {
        ImGui::TextDisabled("Asset no longer exists.");
        return;
    }

    ImGui::TextDisabled("Name"); ImGui::SameLine();
    ImGui::Text("%s", std::filesystem::path(path).filename().string().c_str());
    ImGui::TextDisabled("Type"); ImGui::SameLine();
    ImGui::Text("%s", Vortx::AssetTypeToString(meta.type));
    ImGui::Separator();

    int refCount = Vortx::AssetDatabase::GetRefCount(id);
    ImGui::TextDisabled("Loaded"); ImGui::SameLine();
    if (refCount > 0) ImGui::Text("Yes (refcount %d)", refCount);
    else ImGui::Text("No");

    for (auto& [owner, count] : Vortx::AssetDatabase::GetRefOwners(id))
        ImGui::BulletText("%s: %d", owner.c_str(), count);
    ImGui::Separator();

    auto fn = assetFunc.find(meta.type);
    if (fn != assetFunc.end()) {
        fn->second(id, meta);

        AssetData& data = assetData[meta.type];

        if (data.hasApply)
        {
            ImGui::Separator();
            if (ImGui::Button("Save"))
            {
                AssetDatabase::SaveMeta(id);
            }
        }
    }
}

void AssetPropertiesLoader::RegisterComponent(AssetType type, bool hasApply, AssetPropertiesLoader::DrawFn drawFn)
{
    assetFunc[type] = std::move(drawFn);
    assetData[type] = {hasApply};
}