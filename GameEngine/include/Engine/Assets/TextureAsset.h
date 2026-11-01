#pragma once

#include "EngineAPI.h"
#include "Core/Asset.h"
#include "Core/AssetMeta.h"
#include "Renderer/RenderTypes.h"
#include "EngineAPI.h"

#include <cstdint>
#include <string>

namespace Vortx
{
    class ENGINE_API TextureAsset : public Asset
    {
    public:
        TextureHandle handle;
        int width = 0;
        int height = 0;
        int channels = 0;

        ~TextureAsset() override;

        static Asset* Load(const std::string& path, const AssetMeta& meta);
        static void Unload(TextureAsset* asset);
    };
}