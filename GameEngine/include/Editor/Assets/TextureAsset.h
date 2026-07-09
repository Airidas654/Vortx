#pragma once

#include "EngineAPI.h"
#include "Core/Asset.h"
#include "Core/AssetMeta.h"

#include <cstdint>
#include <string>

namespace Vortx
{
    class TextureAsset : public Asset
    {
    public:
        uint32_t glHandle = 0;
        int width = 0;
        int height = 0;
        int channels = 0;

        ~TextureAsset() override;

        static Asset* Load(const std::string& path, const AssetMeta& meta);
        static void Unload(TextureAsset* asset);

        //static void EnsureRegistered();
    };
}
