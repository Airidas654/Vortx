#include "Assets/TextureAsset.h"
#include "Core/AssetDatabase.h"
#include "SimpleAPI\Renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>

namespace Vortx
{
    TextureAsset::~TextureAsset()
    {
        if (handle.IsValid())
            Renderer::DestroyTexture(handle);
    }

    Asset* TextureAsset::Load(const std::string& path, const AssetMeta& meta)
    {
        bool filterLinear = meta.importSettings.value("filterLinear", true);
        bool wrapRepeat = meta.importSettings.value("wrapRepeat", false);

        stbi_set_flip_vertically_on_load(true);

        int width = 0, height = 0, channels = 0;
        unsigned char* pixels = stbi_load(path.c_str(), &width, &height, &channels, 4);

        if (!pixels)
        {
            std::cerr << "Failed to load '" << path << "': " << stbi_failure_reason() << "\n";
            return nullptr;
        }

        TextureDesc desc;
        desc.width = (uint32_t)width;
        desc.height = (uint32_t)height;
        desc.channels = 4;
        desc.pixels = pixels;
        desc.filter = filterLinear ? TextureFilter::Linear : TextureFilter::Nearest;
        desc.wrap = wrapRepeat ? TextureWrap::Repeat : TextureWrap::ClampToEdge;

        TextureAsset* tex = new TextureAsset();
        tex->width = width;
        tex->height = height;
        tex->channels = 4;
        tex->handle = Renderer::CreateTexture(desc);

        stbi_image_free(pixels);

        if (!tex->handle.IsValid())
        {
            delete tex;
            return nullptr;
        }

        return tex;
    }

    void TextureAsset::Unload(TextureAsset* asset) { delete asset; }

    namespace
    {
        bool RegisterTextureLoader()
        {
            AssetDatabase::RegisterLoader(AssetType::Texture,
                [](const std::string& path, const AssetMeta& meta) -> Asset* { return TextureAsset::Load(path, meta); },
                [](Asset* asset) { TextureAsset::Unload(static_cast<TextureAsset*>(asset)); });
            return true;
        }
        bool textureLoaderRegistered = RegisterTextureLoader();
    }
}