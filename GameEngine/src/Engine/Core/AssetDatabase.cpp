#include "Core/AssetDatabase.h"

#include <filesystem>
#include <iostream>

namespace Vortx
{
    std::unordered_map<std::string, AssetID>& AssetDatabase::PathToGuid()
    {
        static std::unordered_map<std::string, AssetID> value;
        return value;
    }

    std::unordered_map<AssetID, std::string>& AssetDatabase::GuidToPath()
    {
        static std::unordered_map<AssetID, std::string> value;
        return value;
    }

    std::unordered_map<AssetID, AssetMeta>& AssetDatabase::GuidToMeta()
    {
        static std::unordered_map<AssetID, AssetMeta> value;
        return value;
    }

    std::unordered_map<AssetID, AssetDatabase::LoadedEntry>& AssetDatabase::Loaded()
    {
        static std::unordered_map<AssetID, LoadedEntry> value;
        return value;
    }

    std::unordered_map<AssetType, AssetDatabase::LoadFn>& AssetDatabase::LoadFns()
    {
        static std::unordered_map<AssetType, LoadFn> value;
        return value;
    }

    std::unordered_map<AssetType, AssetDatabase::UnloadFn>& AssetDatabase::UnloadFns()
    {
        static std::unordered_map<AssetType, UnloadFn> value;
        return value;
    }

    std::unordered_set<AssetID>& AssetDatabase::ActiveSet()
    {
        static std::unordered_set<AssetID> value;
        return value;
    }

    void AssetDatabase::RegisterLoader(AssetType type, LoadFn load, UnloadFn unload)
    {
        LoadFns()[type] = std::move(load);
        UnloadFns()[type] = std::move(unload);
    }

    void AssetDatabase::Refresh(const std::string& root)
    {
        if (!std::filesystem::exists(root)) return;

        auto& pathToGuid = PathToGuid();
        auto& guidToPath = GuidToPath();
        auto& guidToMeta = GuidToMeta();

        pathToGuid.clear();
        guidToPath.clear();
        guidToMeta.clear();

        for (auto& entry : std::filesystem::recursive_directory_iterator(root))
        {
            if (!entry.is_regular_file()) continue;
            if (entry.path().extension() == ".meta") continue;

            AssetType type = AssetTypeFromExtension(entry.path().extension().string());
            if (type == AssetType::Unknown) continue;

            std::string path = entry.path().string();

            AssetMeta meta = AssetMeta::EnsureMeta(path);

            if (meta.type != type)
            {
                meta.type = type;
                AssetMeta::Save(path + ".meta", meta);
            }

            pathToGuid[path] = meta.guid;
            guidToPath[meta.guid] = path;
            guidToMeta[meta.guid] = meta;
        }
    }

    Asset* AssetDatabase::AcquireInternal(const AssetID& id)
    {
        auto& loaded = Loaded();

        auto it = loaded.find(id);
        if (it != loaded.end())
        {
            it->second.refCount++;
            return it->second.asset;
        }

        auto& guidToPath = GuidToPath();
        auto& guidToMeta = GuidToMeta();

        auto pathIt = guidToPath.find(id);
        auto metaIt = guidToMeta.find(id);
        if (pathIt == guidToPath.end() || metaIt == guidToMeta.end())
        {
            std::cerr << "Unknown asset guid, cannot load\n";
            return nullptr;
        }

        auto& loadFns = LoadFns();
        auto loaderIt = loadFns.find(metaIt->second.type);
        if (loaderIt == loadFns.end())
        {
            std::cerr << "No loader registered for asset type '" << AssetTypeToString(metaIt->second.type) << "'\n";
            return nullptr;
        }

        Asset* asset = loaderIt->second(pathIt->second, metaIt->second);
        if (!asset) return nullptr;

        asset->guid = id;
        asset->type = metaIt->second.type;

        loaded[id] = { asset, 1 };
        return asset;
    }

    Asset* AssetDatabase::GetLoadedInternal(const AssetID& id)
    {
        auto& loaded = Loaded();
        auto it = loaded.find(id);
        return it != loaded.end() ? it->second.asset : nullptr;
    }

    void AssetDatabase::Release(const AssetID& id)
    {
        auto& loaded = Loaded();
        auto it = loaded.find(id);
        if (it == loaded.end()) return;

        it->second.refCount--;
        if (it->second.refCount > 0) return;

        AssetType type = it->second.asset->type;
        auto& unloadFns = UnloadFns();
        auto unloaderIt = unloadFns.find(type);
        if (unloaderIt != unloadFns.end())
            unloaderIt->second(it->second.asset);
        else
            delete it->second.asset;

        loaded.erase(it);
    }

    void AssetDatabase::SyncActiveSet(const std::vector<AssetID>& refs)
    {
        auto& activeSet = ActiveSet();
        std::unordered_set<AssetID> next(refs.begin(), refs.end());

        for (const AssetID& id : next)
            if (activeSet.find(id) == activeSet.end())
                AcquireInternal(id);

        for (const AssetID& id : activeSet)
            if (next.find(id) == next.end())
                Release(id);

        activeSet = std::move(next);
    }

    bool AssetDatabase::TryGetPath(const AssetID& id, std::string& outPath)
    {
        auto& guidToPath = GuidToPath();
        auto it = guidToPath.find(id);
        if (it == guidToPath.end()) return false;
        outPath = it->second;
        return true;
    }

    bool AssetDatabase::TryGetGuid(const std::string& path, AssetID& outGuid)
    {
        auto& pathToGuid = PathToGuid();
        auto it = pathToGuid.find(path);
        if (it == pathToGuid.end()) return false;
        outGuid = it->second;
        return true;
    }

    AssetType AssetDatabase::GetType(const AssetID& id)
    {
        auto& guidToMeta = GuidToMeta();
        auto it = guidToMeta.find(id);
        return it != guidToMeta.end() ? it->second.type : AssetType::Unknown;
    }

    const std::unordered_map<std::string, AssetID>& AssetDatabase::GetAllAssets()
    {
        return PathToGuid();
    }

    void AssetDatabase::DeleteAsset(const AssetID& id)
    {
        auto& guidToPath = GuidToPath();
        auto pathIt = guidToPath.find(id);
        if (pathIt == guidToPath.end()) return;

        auto& loaded = Loaded();
        auto loadedIt = loaded.find(id);
        if (loadedIt != loaded.end())
        {
            AssetType type = loadedIt->second.asset->type;
            auto& unloadFns = UnloadFns();
            auto unloaderIt = unloadFns.find(type);
            if (unloaderIt != unloadFns.end())
                unloaderIt->second(loadedIt->second.asset);
            else
                delete loadedIt->second.asset;

            loaded.erase(loadedIt);
        }

        std::filesystem::remove(pathIt->second);
        std::filesystem::remove(pathIt->second + ".meta");

        PathToGuid().erase(pathIt->second);
        guidToPath.erase(pathIt);
        GuidToMeta().erase(id);
        ActiveSet().erase(id);
    }
}
