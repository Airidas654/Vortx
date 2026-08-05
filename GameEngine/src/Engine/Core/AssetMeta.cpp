#include "Core/AssetMeta.h"

#include <fstream>
#include <filesystem>
#include <iostream>

namespace Vortx
{
    static nlohmann::json GuidToJson(const AssetID& id)
    {
        return { id.value[0], id.value[1] };
    }

    static AssetID JsonToGuid(const nlohmann::json& j)
    {
        return AssetID(j[0].get<uint64_t>(), j[1].get<uint64_t>());
    }

    bool AssetMeta::Load(const std::string& metaPath, AssetMeta& out)
    {
        if (!std::filesystem::exists(metaPath)) return false;

        std::ifstream in(metaPath);
        if (!in.is_open()) return false;

        nlohmann::json j;
        try
        {
            in >> j;
        }
        catch (nlohmann::json::exception& e)
        {
            std::cerr << "Failed to parse '" << metaPath << "': " << e.what() << "\n";
            return false;
        }

        if (!j.contains("guid")) return false;

        out.guid = JsonToGuid(j["guid"]);
        out.type = AssetTypeFromString(j.value("type", std::string("Unknown")));
        out.importSettings = j.value("importSettings", nlohmann::json::object());

        return true;
    }

    void AssetMeta::Save(const std::string& metaPath, const AssetMeta& meta)
    {
        nlohmann::json j;
        j["guid"] = GuidToJson(meta.guid);
        j["type"] = AssetTypeToString(meta.type);
        j["importSettings"] = meta.importSettings;

        std::ofstream out(metaPath);
        out << j.dump(4);
        out.close();
    }

    AssetMeta AssetMeta::EnsureMeta(const std::string& assetPath)
    {
        std::string metaPath = assetPath + ".meta";

        AssetMeta meta;
        if (Load(metaPath, meta))
            return meta;

        meta.guid = AssetID();
        meta.type = AssetTypeFromExtension(std::filesystem::path(assetPath).extension().string());
        Save(metaPath, meta);

        return meta;
    }


}
