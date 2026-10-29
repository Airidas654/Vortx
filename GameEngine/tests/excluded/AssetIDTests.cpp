#include <gtest/gtest.h>
#include "AssetID.h"
#include "Core/Asset.h"

#include <unordered_set>

using namespace Vortx;

TEST(AssetID, ScalarCtorSetsLowWordOnly)
{
    AssetID id(42);
    EXPECT_EQ(id.value[0], 42u);
    EXPECT_EQ(id.value[1], 0u);
}

TEST(AssetID, TwoWordCtor)
{
    AssetID id(1, 2);
    EXPECT_EQ(id.value[0], 1u);
    EXPECT_EQ(id.value[1], 2u);
}

TEST(AssetID, DefaultCtorIsRandomAndUnique)
{
    std::unordered_set<AssetID> seen;
    for (int i = 0; i < 200; i++) seen.insert(AssetID());
    EXPECT_EQ(seen.size(), 200u);
}

TEST(AssetID, DefaultCtorIsNotZeroId)
{
    EXPECT_TRUE(AssetID() != AssetID(0));
}

TEST(AssetID, CopyAndEquality)
{
    AssetID a(5, 6);
    AssetID b(a);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != AssetID(5, 7));
    EXPECT_TRUE(a != AssetID(4, 6));
}

TEST(AssetID, EqualIdsHashEqual)
{
    std::hash<AssetID> h;
    EXPECT_EQ(h(AssetID(9, 10)), h(AssetID(9, 10)));
}

TEST(AssetID, UsableAsUnorderedMapKey)
{
    std::unordered_map<AssetID, int> m;
    m[AssetID(1, 1)] = 7;
    EXPECT_EQ(m.at(AssetID(1, 1)), 7);
    EXPECT_EQ(m.count(AssetID(1, 2)), 0u);
}

// ---------------- AssetType helpers (Core/Asset.h) ----------------

TEST(AssetType, ToString)
{
    EXPECT_STREQ(AssetTypeToString(AssetType::Texture), "Texture");
    EXPECT_STREQ(AssetTypeToString(AssetType::Audio), "Audio");
    EXPECT_STREQ(AssetTypeToString(AssetType::Font), "Font");
    EXPECT_STREQ(AssetTypeToString(AssetType::Scene), "Scene");
    EXPECT_STREQ(AssetTypeToString(AssetType::Unknown), "Unknown");
}

TEST(AssetType, FromStringRoundTrip)
{
    for (AssetType t : { AssetType::Texture, AssetType::Audio, AssetType::Font, AssetType::Scene })
        EXPECT_EQ(AssetTypeFromString(AssetTypeToString(t)), t);

    EXPECT_EQ(AssetTypeFromString("garbage"), AssetType::Unknown);
    EXPECT_EQ(AssetTypeFromString(""), AssetType::Unknown);
}

TEST(AssetType, FromExtension)
{
    for (const char* e : { ".png", ".jpg", ".jpeg", ".bmp", ".tga" })
        EXPECT_EQ(AssetTypeFromExtension(e), AssetType::Texture) << e;
    for (const char* e : { ".wav", ".ogg", ".mp3" })
        EXPECT_EQ(AssetTypeFromExtension(e), AssetType::Audio) << e;
    for (const char* e : { ".ttf", ".otf" })
        EXPECT_EQ(AssetTypeFromExtension(e), AssetType::Font) << e;
    EXPECT_EQ(AssetTypeFromExtension(".scene"), AssetType::Scene);
    EXPECT_EQ(AssetTypeFromExtension(".txt"), AssetType::Unknown);
    EXPECT_EQ(AssetTypeFromExtension(""), AssetType::Unknown);
}

TEST(AssetType, FromExtensionIsCaseInsensitive)
{
    EXPECT_EQ(AssetTypeFromExtension(".PNG"), AssetType::Texture);
    EXPECT_EQ(AssetTypeFromExtension(".WaV"), AssetType::Audio);
    EXPECT_EQ(AssetTypeFromExtension(".SCENE"), AssetType::Scene);
}
