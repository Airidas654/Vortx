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
    // Holds decoded PCM + format info. There's deliberately no playback code
    // here - that's a separate AudioSource/mixer concern. This just makes
    // sure "how does an audio file get loaded and refcounted" is handled.
    class ENGINE_API AudioAsset : public Asset
    {
    public:
        std::vector<uint8_t> pcmData;
        uint16_t channels = 0;
        uint32_t sampleRate = 0;
        uint16_t bitsPerSample = 0;

        static Asset* Load(const std::string& path, const AssetMeta& meta);
        static void Unload(AudioAsset* asset);

        //static void EnsureRegistered();
    };
}
