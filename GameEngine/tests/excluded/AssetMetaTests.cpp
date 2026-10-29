#include "TestUtils.h"
#include "Core/AssetMeta.h"

using namespace Vortx;
using TestUtils::TempDir;
using TestUtils::WriteFile;

TEST(AssetMeta, SaveLoadRoundTrip)
{
    TempDir tmp;
    auto path = (tmp / "x.meta").string();

    AssetMeta in;
    in.guid = AssetID(11, 22);
    in.type = AssetType::Texture;
    in.importSettings["filterLinear"] = false;
    AssetMeta::Save(path, in);

    AssetMeta out;
    ASSERT_TRUE(AssetMeta::Load(path, out));
    EXPECT_TRUE(out.guid == AssetID(11, 22));
    EXPECT_EQ(out.type, AssetType::Texture);
    EXPECT_FALSE(out.importSettings.value("filterLinear", true));
}

TEST(AssetMeta, LoadMissingFileFails)
{
    TempDir tmp;
    AssetMeta out;
    EXPECT_FALSE(AssetMeta::Load((tmp / "nope.meta").string(), out));
}

TEST(AssetMeta, LoadInvalidJsonFails)
{
    TempDir tmp;
    WriteFile(tmp / "bad.meta", "{ not json");
    AssetMeta out;
    EXPECT_FALSE(AssetMeta::Load((tmp / "bad.meta").string(), out));
}

TEST(AssetMeta, LoadWithoutGuidFails)
{
    TempDir tmp;
    WriteFile(tmp / "noguid.meta", R"({"type":"Texture"})");
    AssetMeta out;
    EXPECT_FALSE(AssetMeta::Load((tmp / "noguid.meta").string(), out));
}

TEST(AssetMeta, LoadMissingOptionalFieldsUsesDefaults)
{
    TempDir tmp;
    WriteFile(tmp / "min.meta", R"({"guid":[1,2]})");
    AssetMeta out;
    ASSERT_TRUE(AssetMeta::Load((tmp / "min.meta").string(), out));
    EXPECT_EQ(out.type, AssetType::Unknown);
    EXPECT_TRUE(out.importSettings.is_object());
    EXPECT_TRUE(out.importSettings.empty());
}

TEST(AssetMeta, EnsureMetaCreatesFileWithTypeFromExtension)
{
    TempDir tmp;
    auto asset = tmp / "tex.png";
    WriteFile(asset, "x");

    AssetMeta meta = AssetMeta::EnsureMeta(asset.string());
    EXPECT_EQ(meta.type, AssetType::Texture);
    EXPECT_TRUE(std::filesystem::exists(asset.string() + ".meta"));
}

TEST(AssetMeta, EnsureMetaIsStableAcrossCalls)
{
    TempDir tmp;
    auto asset = tmp / "sound.wav";
    WriteFile(asset, "x");

    AssetMeta first = AssetMeta::EnsureMeta(asset.string());
    AssetMeta second = AssetMeta::EnsureMeta(asset.string());
    EXPECT_TRUE(first.guid == second.guid);
}
