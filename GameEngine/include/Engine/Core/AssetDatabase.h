#pragma once

#include "EngineAPI.h"
#include "AssetID.h"
#include "Core/Asset.h"
#include "Core/AssetMeta.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>

namespace Vortx
{
    class ENGINE_API AssetDatabase
    {
    public:
        typedef std::function<Asset*(const std::string& path, const AssetMeta& meta)> LoadFn;
        typedef std::function<void(Asset*)> UnloadFn;

        static void Refresh(const std::string& root);

        static void RegisterLoader(AssetType type, LoadFn load, UnloadFn unload);

        template<typename T>
        static T* Acquire(const AssetID& id)
        {
            return static_cast<T*>(AcquireInternal(id));
        }

        static void Release(const AssetID& id);

        template<typename T>
        static T* GetLoaded(const AssetID& id)
        {
            return static_cast<T*>(GetLoadedInternal(id));
        }

        static void SyncActiveSet(const std::vector<AssetID>& refs);

        static bool TryGetPath(const AssetID& id, std::string& outPath);
        static bool TryGetGuid(const std::string& path, AssetID& outGuid);
        static AssetType GetType(const AssetID& id);

        static const std::unordered_map<std::string, AssetID>& GetAllAssets();

        static void DeleteAsset(const AssetID& id);

    private:
        static Asset* AcquireInternal(const AssetID& id);
        static Asset* GetLoadedInternal(const AssetID& id);

        struct LoadedEntry
        {
            Asset* asset = nullptr;
            int refCount = 0;
        };

        static std::unordered_map<std::string, AssetID>& PathToGuid();
        static std::unordered_map<AssetID, std::string>& GuidToPath();
        static std::unordered_map<AssetID, AssetMeta>& GuidToMeta();
        static std::unordered_map<AssetID, LoadedEntry>& Loaded();
        static std::unordered_map<AssetType, LoadFn>& LoadFns();
        static std::unordered_map<AssetType, UnloadFn>& UnloadFns();
        static std::unordered_set<AssetID>& ActiveSet();
    };
}
