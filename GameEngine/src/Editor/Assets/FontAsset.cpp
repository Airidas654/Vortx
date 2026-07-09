#include "Assets/FontAsset.h"
#include "Core/AssetDatabase.h"

#include <fstream>
#include <iostream>
#include <iterator>

namespace Vortx
{
    Asset* FontAsset::Load(const std::string& path, const AssetMeta& meta)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "[FontAsset] Failed to open '" << path << "'\n";
            return nullptr;
        }

        FontAsset* font = new FontAsset();
        font->fileData = std::vector<uint8_t>(std::istreambuf_iterator<char>(file), {});

        return font;
    }

    void FontAsset::Unload(FontAsset* asset)
    {
        delete asset;
    }

    //void FontAsset::EnsureRegistered() {}

    namespace
    {
        bool RegisterFontLoader()
        {
            AssetDatabase::RegisterLoader(AssetType::Font,
                [](const std::string& path, const AssetMeta& meta) -> Asset*
                {
                    return FontAsset::Load(path, meta);
                },
                [](Asset* asset)
                {
                    FontAsset::Unload(static_cast<FontAsset*>(asset));
                });
            return true;
        }

        bool fontLoaderRegistered = RegisterFontLoader();
    }
}
