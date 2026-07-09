#include "Assets/AudioAsset.h"
#include "Core/AssetDatabase.h"

#include <fstream>
#include <cstring>
#include <iostream>
#include <filesystem>

namespace Vortx
{
    namespace
    {
        struct ChunkHeader { char id[4]; uint32_t size; };

        // Minimal RIFF/WAVE parser - no external dependency needed for the
        // common uncompressed-PCM .wav case. Assumes a little-endian host.
        bool LoadWav(const std::string& path, AudioAsset& out)
        {
            std::ifstream file(path, std::ios::binary);
            if (!file.is_open()) return false;

            char riff[4];
            file.read(riff, 4);
            if (std::strncmp(riff, "RIFF", 4) != 0) return false;

            file.ignore(4); // overall chunk size, unused

            char wave[4];
            file.read(wave, 4);
            if (std::strncmp(wave, "WAVE", 4) != 0) return false;

            bool haveFmt = false;

            while (file.good())
            {
                ChunkHeader header{};
                file.read(header.id, 4);
                file.read(reinterpret_cast<char*>(&header.size), 4);
                if (!file.good()) break;

                if (std::strncmp(header.id, "fmt ", 4) == 0)
                {
                    uint16_t audioFormat = 0;
                    file.read(reinterpret_cast<char*>(&audioFormat), 2);
                    file.read(reinterpret_cast<char*>(&out.channels), 2);
                    file.read(reinterpret_cast<char*>(&out.sampleRate), 4);
                    file.ignore(6); // byteRate (4) + blockAlign (2)
                    file.read(reinterpret_cast<char*>(&out.bitsPerSample), 2);

                    const uint32_t consumed = 16;
                    if (header.size > consumed)
                        file.ignore(header.size - consumed);

                    haveFmt = true;
                }
                else if (std::strncmp(header.id, "data", 4) == 0)
                {
                    out.pcmData.resize(header.size);
                    file.read(reinterpret_cast<char*>(out.pcmData.data()), header.size);
                }
                else
                {
                    file.ignore(header.size);
                }

                if (header.size % 2 != 0) file.ignore(1); // chunks are word-aligned
            }

            return haveFmt && !out.pcmData.empty();
        }
    }

    Asset* AudioAsset::Load(const std::string& path, const AssetMeta& meta)
    {
        std::string ext = std::filesystem::path(path).extension().string();

        if (ext != ".wav")
        {
            std::cerr << "[AudioAsset] Only .wav is supported out of the box ('" << path
                       << "'). Plug a decoder (e.g. stb_vorbis for .ogg, minimp3 for .mp3) "
                       << "into AudioAsset::Load for other formats.\n";
            return nullptr;
        }

        AudioAsset* audio = new AudioAsset();
        if (!LoadWav(path, *audio))
        {
            std::cerr << "[AudioAsset] Failed to parse '" << path << "'\n";
            delete audio;
            return nullptr;
        }

        return audio;
    }

    void AudioAsset::Unload(AudioAsset* asset)
    {
        delete asset;
    }

    //void AudioAsset::EnsureRegistered() {}

    namespace
    {
        bool RegisterAudioLoader()
        {
            AssetDatabase::RegisterLoader(AssetType::Audio,
                [](const std::string& path, const AssetMeta& meta) -> Asset*
                {
                    return AudioAsset::Load(path, meta);
                },
                [](Asset* asset)
                {
                    AudioAsset::Unload(static_cast<AudioAsset*>(asset));
                });
            return true;
        }

        bool audioLoaderRegistered = RegisterAudioLoader();
    }
}
