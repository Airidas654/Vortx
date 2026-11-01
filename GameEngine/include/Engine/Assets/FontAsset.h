#pragma once

#include "EngineAPI.h"
#include "Core/Asset.h"
#include "Core/AssetMeta.h"
#include "EngineAPI.h"

#include <vector>
#include <cstdint>
#include <string>

namespace Vortx
{
    // Minimal on purpose: holds the raw font file bytes so a future text
    // renderer can feed them into stb_truetype (or similar) on demand. It
    // still participates fully in the refcounted load/unload system - the
    // rasterization/atlas step is a separate concern you can layer on top
    // whenever you build text rendering.
    class ENGINE_API FontAsset : public Asset
    {
    public:
        std::vector<uint8_t> fileData;

        static Asset* Load(const std::string& path, const AssetMeta& meta);
        static void Unload(FontAsset* asset);

       // static void EnsureRegistered();
    };
}
