#include "Core/Asset.h"

#include <algorithm>
#include <cctype>

namespace Vortx
{
    const char* AssetTypeToString(AssetType type)
    {
        switch (type)
        {
            case AssetType::Texture:
                return "Texture";
            case AssetType::Audio:
                return "Audio";
            case AssetType::Font:
                return "Font";
            case AssetType::Scene:
                return "Scene";
            default:
                return "Unknown";
        }
    }

    AssetType AssetTypeFromString(const std::string& str)
    {
        if (str == "Texture") return AssetType::Texture;
        if (str == "Audio") return AssetType::Audio;
        if (str == "Font") return AssetType::Font;
        if (str == "Scene") return AssetType::Scene;
        return AssetType::Unknown;
    }

    AssetType AssetTypeFromExtension(const std::string& extension)
    {
        std::string ext = extension;
        std::transform(ext.begin(), ext.end(), ext.begin(),
            [](unsigned char c) { return (char)std::tolower(c); });

        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga")
            return AssetType::Texture;

        if (ext == ".wav" || ext == ".ogg" || ext == ".mp3")
            return AssetType::Audio;

        if (ext == ".ttf" || ext == ".otf")
            return AssetType::Font;

        if (ext == ".scene")
            return AssetType::Scene;

        return AssetType::Unknown;
    }
}
