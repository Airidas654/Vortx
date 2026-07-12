#include "Assets/TextureAsset.h"
#include "Core/AssetDatabase.h"

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>

namespace Vortx
{
    TextureAsset::~TextureAsset()
    {
        if (glHandle)
            glDeleteTextures(1, &glHandle);
    }

    Asset* TextureAsset::Load(const std::string& path, const AssetMeta& meta)
    {
        bool srgb = meta.importSettings.value("srgb", true);
        bool filterLinear = meta.importSettings.value("filterLinear", true);
        bool generateMips = meta.importSettings.value("generateMipmaps", false);

        stbi_set_flip_vertically_on_load(true);

        int width = 0, height = 0, channels = 0;
        unsigned char* pixels = stbi_load(path.c_str(), &width, &height, &channels, 4);

        if (!pixels)
        {
            std::cerr << "Failed to load '" << path << "': " << stbi_failure_reason() << "\n";
            return nullptr;
        }

        TextureAsset* tex = new TextureAsset();
        tex->width = width;
        tex->height = height;
        tex->channels = 4;

        glGenTextures(1, &tex->glHandle);
        glBindTexture(GL_TEXTURE_2D, tex->glHandle);

        GLenum internalFormat = srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

        GLenum filter = filterLinear ? GL_LINEAR : GL_NEAREST;
        GLenum minFilter = filter;
        if (generateMips)
            minFilter = filterLinear ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST;

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        if (generateMips)
            glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(pixels);

        return tex;
    }

    void TextureAsset::Unload(TextureAsset* asset)
    {
        delete asset;
    }

    //void TextureAsset::EnsureRegistered() {}

    namespace
    {
        bool RegisterTextureLoader()
        {
            AssetDatabase::RegisterLoader(AssetType::Texture,
                [](const std::string& path, const AssetMeta& meta) -> Asset*
                {
                    return TextureAsset::Load(path, meta);
                },
                [](Asset* asset)
                {
                    TextureAsset::Unload(static_cast<TextureAsset*>(asset));
                });
            return true;
        }

        bool textureLoaderRegistered = RegisterTextureLoader();
    }
}
