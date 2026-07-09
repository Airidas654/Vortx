#pragma once

#include "EngineAPI.h"
#include "AssetID.h"

#include <cstdint>
#include <string>

namespace Vortx
{
    enum class AssetType : uint32_t
    {
        Unknown = 0,
        Texture,
        Audio,
        Font
    };

    ENGINE_API const char* AssetTypeToString(AssetType type);
    ENGINE_API AssetType AssetTypeFromString(const std::string& str);
    ENGINE_API AssetType AssetTypeFromExtension(const std::string& extension);

    class ENGINE_API Asset
    {
    public:
        AssetID guid;
        AssetType type = AssetType::Unknown;

        virtual ~Asset() = default;
    };
}
