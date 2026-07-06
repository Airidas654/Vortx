#pragma once

#include "EngineAPI.h"

#include <cstdint>
#include <random>

namespace Vortx
{

    struct ENGINE_API AssetID
    {
        uint64_t value[2];

        AssetID() {
            static std::random_device rd;
            static std::mt19937_64 gen(rd());
            static std::uniform_int_distribution<uint64_t> dist;

            value[0] = dist(gen);
            value[1] = dist(gen);
        }

        AssetID(uint64_t id) {
            value[0] = id;
            value[1] = 0;
        }

        AssetID(uint64_t id0, uint64_t id1) {
            value[0] = id0;
            value[1] = id1;
        }

        AssetID(const AssetID& assetId) {
            value[0] = assetId.value[0];
            value[1] = assetId.value[1];
        }

        bool operator==(const AssetID& other) const
        {
            return value[0] == other.value[0] && value[1] == other.value[1];
        }

        bool operator!=(const AssetID& other) const
        {
            return value[0] != other.value[0] || value[1] != other.value[1];
        }
    };
}
template <>
struct ENGINE_API std::hash<Vortx::AssetID>
{
    std::size_t operator()(const Vortx::AssetID& k) const
    {
        return (hash<uint64_t>()(k.value[0])) ^ (hash<uint64_t>()(k.value[1]));
    }
};

