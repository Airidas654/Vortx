#include "TestUtils.h"
#include "Core/AssetDatabase.h"
#include "Assets/FontAsset.h"
#include "Components/ComponentSpriteRenderer.h"

#include "entt.hpp"
#include "json.hpp"

using namespace Vortx;
using namespace TestUtils;

namespace
{
    int g_loads = 0;
    int g_unloads = 0;

    Asset* FakeLoad(const std::string&, const AssetMeta&) { ++g_loads; return new Asset(); }
    void FakeUnload(Asset* a) { ++g_unloads; delete a; }
}

// Uses AssetType::Font with a counting fake loader, so no GPU is needed.
class AssetDatabaseTest : public ::testing::Test
{
protected:
    TempDir tmp;
    fs::path assets;

    void SetUp() override
    {
        g_loads = g_unloads = 0;
        AssetDatabase::RegisterLoader(AssetType::Font, FakeLoad, FakeUnload);

        assets = tmp.path() / "Assets";
        WriteFile(assets / "a.ttf", "aaa");
        WriteFile(assets / "b.otf", "bbb");
        WriteFile(assets / "notes.txt", "ignored");
        AssetDatabase::Refresh(assets.string());
    }

    void TearDown() override
    {
        AssetDatabase::UnloadAll();
        AssetDatabase::RegisterLoader(AssetType::Font,
            [](const std::string& p, const AssetMeta& m) -> Asset* { return FontAsset::Load(p, m); },
            [](Asset* a) { FontAsset::Unload(static_cast<FontAsset*>(a)); });
    }

    AssetID GuidOf(const std::string& file)
    {
        AssetID id(0);
        EXPECT_TRUE(AssetDatabase::TryGetGuid((assets / file).string(), id)) << file;
        return id;
    }
};

TEST_F(AssetDatabaseTest, RefreshRegistersKnownAssetsOnly)
{
    EXPECT_EQ(AssetDatabase::GetAllAssets().size(), 2u);

    AssetID dummy(0);
    EXPECT_FALSE(AssetDatabase::TryGetGuid((assets / "notes.txt").string(), dummy));
    EXPECT_TRUE(fs::exists(assets / "a.ttf.meta"));
}

TEST_F(AssetDatabaseTest, RefreshReportsTypeAndPath)
{
    AssetID id = GuidOf("a.ttf");
    EXPECT_EQ(AssetDatabase::GetType(id), AssetType::Font);

    std::string path;
    ASSERT_TRUE(AssetDatabase::TryGetPath(id, path));
    EXPECT_EQ(path, (assets / "a.ttf").string());
}

TEST_F(AssetDatabaseTest, RefreshKeepsGuidsStable)
{
    AssetID before = GuidOf("a.ttf");
    AssetDatabase::Refresh(assets.string());
    EXPECT_TRUE(before == GuidOf("a.ttf"));
}

TEST_F(AssetDatabaseTest, RefreshOnMissingRootIsNoOp)
{
    AssetDatabase::Refresh((tmp / "does_not_exist").string());
    EXPECT_EQ(AssetDatabase::GetAllAssets().size(), 2u);
}

TEST_F(AssetDatabaseTest, RefreshWithDifferentRootUnloadsEverything)
{
    AssetID id = GuidOf("a.ttf");
    ASSERT_NE(AssetDatabase::Acquire<Asset>(id, "T"), nullptr);

    fs::create_directories(tmp / "Other");
    AssetDatabase::Refresh((tmp / "Other").string());

    EXPECT_EQ(g_unloads, 1);
    EXPECT_EQ(AssetDatabase::GetRefCount(id), 0);
    EXPECT_TRUE(AssetDatabase::GetAllAssets().empty());
}

TEST_F(AssetDatabaseTest, UnknownTypeOrMissingGuidQueries)
{
    EXPECT_EQ(AssetDatabase::GetType(AssetID(123, 456)), AssetType::Unknown);
    std::string p;
    EXPECT_FALSE(AssetDatabase::TryGetPath(AssetID(123, 456), p));
    AssetMeta m;
    EXPECT_FALSE(AssetDatabase::TryGetMeta(AssetID(123, 456), m));
}

TEST_F(AssetDatabaseTest, AcquireLoadsOnceAndRefCounts)
{
    AssetID id = GuidOf("a.ttf");

    Asset* first = AssetDatabase::Acquire<Asset>(id, "Owner1");
    Asset* second = AssetDatabase::Acquire<Asset>(id, "Owner2");
    AssetDatabase::Acquire<Asset>(id, "Owner1");

    ASSERT_NE(first, nullptr);
    EXPECT_EQ(first, second);
    EXPECT_EQ(g_loads, 1);
    EXPECT_EQ(AssetDatabase::GetRefCount(id), 3);

    auto owners = AssetDatabase::GetRefOwners(id);
    EXPECT_EQ(owners["Owner1"], 2);
    EXPECT_EQ(owners["Owner2"], 1);
    EXPECT_TRUE(first->guid == id);
    EXPECT_EQ(first->type, AssetType::Font);
}

TEST_F(AssetDatabaseTest, ReleaseUnloadsAtZero)
{
    AssetID id = GuidOf("a.ttf");
    AssetDatabase::Acquire<Asset>(id, "A");
    AssetDatabase::Acquire<Asset>(id, "B");

    AssetDatabase::Release(id, "A");
    EXPECT_EQ(g_unloads, 0);
    EXPECT_NE(AssetDatabase::GetLoaded<Asset>(id), nullptr);

    AssetDatabase::Release(id, "B");
    EXPECT_EQ(g_unloads, 1);
    EXPECT_EQ(AssetDatabase::GetLoaded<Asset>(id), nullptr);
    EXPECT_EQ(AssetDatabase::GetRefCount(id), 0);
}

TEST_F(AssetDatabaseTest, ReleaseOfUnloadedAssetIsHarmless)
{
    AssetDatabase::Release(GuidOf("a.ttf"), "Nobody");
    EXPECT_EQ(g_unloads, 0);
}

TEST_F(AssetDatabaseTest, AcquireUnknownGuidReturnsNull)
{
    EXPECT_EQ(AssetDatabase::Acquire<Asset>(AssetID(9, 9), "T"), nullptr);
}

TEST_F(AssetDatabaseTest, AcquireWithoutLoaderReturnsNull)
{
    // Scene type has no loader registered in this test binary.
    WriteFile(assets / "level.scene", "x");
    AssetDatabase::Refresh(assets.string());
    EXPECT_EQ(AssetDatabase::Acquire<Asset>(GuidOf("level.scene"), "T"), nullptr);
}

TEST_F(AssetDatabaseTest, PinAndUnpinBehaveLikeAcquireRelease)
{
    AssetID id = GuidOf("b.otf");
    AssetDatabase::Pin(id, "Pinner");
    EXPECT_EQ(AssetDatabase::GetRefCount(id), 1);
    AssetDatabase::Unpin(id, "Pinner");
    EXPECT_EQ(AssetDatabase::GetRefCount(id), 0);
}

TEST_F(AssetDatabaseTest, SyncActiveSetAcquiresAndReleasesDiff)
{
    AssetID a = GuidOf("a.ttf");
    AssetID b = GuidOf("b.otf");

    AssetDatabase::SyncActiveSet({ a });
    EXPECT_EQ(AssetDatabase::GetRefCount(a), 1);
    EXPECT_EQ(g_loads, 1);

    AssetDatabase::SyncActiveSet({ a, b });
    EXPECT_EQ(AssetDatabase::GetRefCount(a), 1);   // not re-acquired
    EXPECT_EQ(AssetDatabase::GetRefCount(b), 1);
    EXPECT_EQ(g_loads, 2);

    AssetDatabase::SyncActiveSet({ b });
    EXPECT_EQ(AssetDatabase::GetRefCount(a), 0);
    EXPECT_EQ(g_unloads, 1);

    AssetDatabase::SyncActiveSet({});
    EXPECT_EQ(AssetDatabase::GetRefCount(b), 0);
}

TEST_F(AssetDatabaseTest, SetImportSettingsIsInMemoryUntilSaved)
{
    AssetID id = GuidOf("a.ttf");
    AssetDatabase::SetImportSettings(id, nlohmann::json{ {"k", 1} });

    AssetMeta meta;
    ASSERT_TRUE(AssetDatabase::TryGetMeta(id, meta));
    EXPECT_EQ(meta.importSettings["k"], 1);

    AssetMeta onDisk;
    ASSERT_TRUE(AssetMeta::Load((assets / "a.ttf.meta").string(), onDisk));
    EXPECT_FALSE(onDisk.importSettings.contains("k"));

    AssetDatabase::SaveMeta(id);
    ASSERT_TRUE(AssetMeta::Load((assets / "a.ttf.meta").string(), onDisk));
    EXPECT_EQ(onDisk.importSettings["k"], 1);
}

TEST_F(AssetDatabaseTest, ReimportReloadsLoadedAsset)
{
    AssetID id = GuidOf("a.ttf");
    AssetDatabase::Acquire<Asset>(id, "T");

    AssetDatabase::Reimport(id);

    EXPECT_EQ(g_loads, 2);
    EXPECT_EQ(g_unloads, 1);
    EXPECT_EQ(AssetDatabase::GetRefCount(id), 1);
    EXPECT_NE(AssetDatabase::GetLoaded<Asset>(id), nullptr);
}

TEST_F(AssetDatabaseTest, ReimportOfUnloadedAssetDoesNothing)
{
    AssetDatabase::Reimport(GuidOf("a.ttf"));
    EXPECT_EQ(g_loads, 0);
}

TEST_F(AssetDatabaseTest, DeleteAssetRemovesFilesAndEntries)
{
    AssetID id = GuidOf("a.ttf");
    AssetDatabase::Acquire<Asset>(id, "T");

    AssetDatabase::DeleteAsset(id);

    EXPECT_FALSE(fs::exists(assets / "a.ttf"));
    EXPECT_FALSE(fs::exists(assets / "a.ttf.meta"));
    EXPECT_EQ(g_unloads, 1);
    EXPECT_EQ(AssetDatabase::GetLoaded<Asset>(id), nullptr);
    std::string p;
    EXPECT_FALSE(AssetDatabase::TryGetPath(id, p));
    EXPECT_EQ(AssetDatabase::GetAllAssets().size(), 1u);
}

TEST_F(AssetDatabaseTest, UnloadAllReleasesEverything)
{
    AssetDatabase::Acquire<Asset>(GuidOf("a.ttf"), "T");
    AssetDatabase::Acquire<Asset>(GuidOf("b.otf"), "T");
    AssetDatabase::UnloadAll();
    EXPECT_EQ(g_unloads, 2);
    EXPECT_EQ(AssetDatabase::GetRefCount(GuidOf("a.ttf")), 0);
}

TEST_F(AssetDatabaseTest, PreloadSceneAcquiresReferencedAssetsAndReleases)
{
    SerializerLoader::Reset();
    AssetID id = GuidOf("a.ttf");

    nlohmann::json comp = { {"textureID", nlohmann::json::array({ id.value[0], id.value[1] })} };
    nlohmann::json entity;
    entity["components"][std::to_string(entt::type_hash<SpriteRendererData>::value())] = comp;
    nlohmann::json root;
    root["entities"] = nlohmann::json::array({ entity });

    auto scenePath = (tmp / "level.scene").string();
    WriteBytes(scenePath, nlohmann::json::to_cbor(root));

    EXPECT_FALSE(AssetDatabase::IsScenePreloaded(scenePath));
    AssetDatabase::PreloadScene(scenePath);
    EXPECT_TRUE(AssetDatabase::IsScenePreloaded(scenePath));
    EXPECT_EQ(AssetDatabase::GetRefCount(id), 1);

    AssetDatabase::PreloadScene(scenePath);   // idempotent
    EXPECT_EQ(AssetDatabase::GetRefCount(id), 1);

    AssetDatabase::ReleasePreloadedScene(scenePath);
    EXPECT_FALSE(AssetDatabase::IsScenePreloaded(scenePath));
    EXPECT_EQ(AssetDatabase::GetRefCount(id), 0);
}
