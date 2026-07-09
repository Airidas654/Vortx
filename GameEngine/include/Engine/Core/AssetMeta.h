#pragma once

#include "Core/Asset.h"
#include "AssetID.h"

#include "json.hpp"

#include <string>

namespace Vortx
{
    struct AssetMeta
    {
        AssetID guid;
        AssetType type = AssetType::Unknown;
        nlohmann::json importSettings = nlohmann::json::object();

        static bool Load(const std::string& metaPath, AssetMeta& out);
        static void Save(const std::string& metaPath, const AssetMeta& meta);

        static AssetMeta EnsureMeta(const std::string& assetPath);
    };
}
