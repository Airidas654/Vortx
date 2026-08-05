#include "Core/AssetDatabase.h"

#include "SerializerLoader.h"

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

        std::string& currentRoot = CurrentRoot();
        if (!currentRoot.empty() && currentRoot != root)
            UnloadAll();
        currentRoot = root;

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

    Asset* AssetDatabase::GetLoadedInternal(const AssetID& id)
    {
        auto& loaded = Loaded();
        auto it = loaded.find(id);
        return it != loaded.end() ? it->second.asset : nullptr;
    }

    Asset* AssetDatabase::AcquireInternal(const AssetID& id, const char* owner)
    {
        auto& loaded = Loaded();

        auto it = loaded.find(id);
        if (it != loaded.end())
        {
            it->second.refCount++;
            it->second.owners[owner]++;
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

        auto loaderIt = LoadFns().find(metaIt->second.type);
        if (loaderIt == LoadFns().end())
        {
            std::cerr << "No loader registered for asset type '" << AssetTypeToString(metaIt->second.type) << "'\n";
            return nullptr;
        }

        Asset* asset = loaderIt->second(pathIt->second, metaIt->second);
        if (!asset) return nullptr;

        asset->guid = id;
        asset->type = metaIt->second.type;

        LoadedEntry entry;
        entry.asset = asset;
        entry.refCount = 1;
        entry.owners[owner] = 1;
        loaded[id] = std::move(entry);
        return asset;
    }

    void AssetDatabase::Release(const AssetID& id, const char* owner)
    {
        auto& loaded = Loaded();
        auto it = loaded.find(id);
        if (it == loaded.end()) return;

        it->second.refCount--;

        auto ownerIt = it->second.owners.find(owner);
        if (ownerIt != it->second.owners.end())
        {
            if (--ownerIt->second <= 0)
                it->second.owners.erase(ownerIt);
        }

        if (it->second.refCount > 0) return;

        AssetType type = it->second.asset->type;
        auto unloaderIt = UnloadFns().find(type);
        if (unloaderIt != UnloadFns().end())
            unloaderIt->second(it->second.asset);
        else
            delete it->second.asset;

        loaded.erase(it);
    }

    std::unordered_map<std::string, int> AssetDatabase::GetRefOwners(const AssetID& id)
    {
        auto it = Loaded().find(id);
        return it != Loaded().end() ? it->second.owners : std::unordered_map<std::string, int>{};
    }

    void AssetDatabase::SyncActiveSet(const std::vector<AssetID>& refs)
    {
        auto& activeSet = ActiveSet();
        std::unordered_set<AssetID> next;

        for (const AssetID& id : refs)
        {
            if (activeSet.find(id) != activeSet.end())
            {
                next.insert(id);
                continue;
            }
            if (AcquireInternal(id, "Scene"))
                next.insert(id);
        }

        for (const AssetID& id : activeSet)
            if (next.find(id) == next.end())
                Release(id, "Scene");

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

    std::string& AssetDatabase::CurrentRoot()
    {
        static std::string value;
        return value;
    }

    void AssetDatabase::UnloadAll()
    {
        auto& loaded = Loaded();
        auto& unloadFns = UnloadFns();

        for (auto& [id, entry] : loaded)
        {
            auto it = unloadFns.find(entry.asset->type);
            if (it != unloadFns.end()) it->second(entry.asset);
            else delete entry.asset;
        }

        loaded.clear();
        ActiveSet().clear();
    }

    bool AssetDatabase::TryGetMeta(const AssetID& id, AssetMeta& outMeta)
    {
        auto& guidToMeta = GuidToMeta();
        auto it = guidToMeta.find(id);
        if (it == guidToMeta.end()) return false;
        outMeta = it->second;
        return true;
    }

    void AssetDatabase::SaveMeta(const AssetID& id)
    {
        auto& guidToMeta = GuidToMeta();
        auto it = guidToMeta.find(id);
        if (it == guidToMeta.end()) return;

        std::string path;
        if (TryGetPath(id, path))
            AssetMeta::Save(path + ".meta", it->second);

        Reimport(id);
    }

    void AssetDatabase::SetImportSettings(const AssetID& id, const nlohmann::json& settings)
    {
        auto& guidToMeta = GuidToMeta();
        auto it = guidToMeta.find(id);
        if (it == guidToMeta.end()) return;

        it->second.importSettings = settings;

        /*std::string path;
        if (TryGetPath(id, path))
            AssetMeta::Save(path + ".meta", it->second);

        Reimport(id);*/
    }

    void AssetDatabase::Reimport(const AssetID& id)
    {
        auto& loaded = Loaded();
        auto it = loaded.find(id);
        if (it == loaded.end()) return;

        auto& guidToPath = GuidToPath();
        auto& guidToMeta = GuidToMeta();
        auto pathIt = guidToPath.find(id);
        auto metaIt = guidToMeta.find(id);
        if (pathIt == guidToPath.end() || metaIt == guidToMeta.end()) return;

        auto loaderIt = LoadFns().find(metaIt->second.type);
        if (loaderIt == LoadFns().end()) return;

        Asset* fresh = loaderIt->second(pathIt->second, metaIt->second);
        if (!fresh) return;
        fresh->guid = id;
        fresh->type = metaIt->second.type;

        auto unloaderIt = UnloadFns().find(it->second.asset->type);
        if (unloaderIt != UnloadFns().end()) unloaderIt->second(it->second.asset);
        else delete it->second.asset;

        it->second.asset = fresh;
    }

    std::unordered_map<std::string, std::vector<AssetID>>& AssetDatabase::PreloadedScenes()
    {
        static std::unordered_map<std::string, std::vector<AssetID>> value;
        return value;
    }

    void AssetDatabase::Pin(const AssetID& id, const char* owner)
    {
        AcquireInternal(id, owner);
    }

    void AssetDatabase::Unpin(const AssetID& id, const char* owner)
    {
        Release(id, owner);
    }

    void AssetDatabase::PreloadScene(const std::string& scenePath)
    {
        if (PreloadedScenes().count(scenePath)) return;

        std::vector<AssetID> refs = SerializerLoader::CollectSceneAssetsFromFile(scenePath);
        std::string owner = "Preload:" + scenePath;
        for (const AssetID& id : refs)
            Pin(id, owner.c_str());

        PreloadedScenes()[scenePath] = std::move(refs);
    }

    void AssetDatabase::ReleasePreloadedScene(const std::string& scenePath)
    {
        auto it = PreloadedScenes().find(scenePath);
        if (it == PreloadedScenes().end()) return;

        std::string owner = "Preload:" + scenePath;
        for (const AssetID& id : it->second)
            Unpin(id, owner.c_str());

        PreloadedScenes().erase(it);
    }

    bool AssetDatabase::IsScenePreloaded(const std::string& scenePath)
    {
        return PreloadedScenes().count(scenePath) != 0;
    }

    int AssetDatabase::GetRefCount(const AssetID& id)
    {
        auto it = Loaded().find(id);

        if (it != Loaded().end()) {
            return it->second.refCount;
        }

        return 0;
    }
}
