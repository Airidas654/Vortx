#include "TestUtils.h"
#include "Components/ComponentBoxCollider.h"
#include "Components/ComponentCamera.h"
#include "Components/ComponentCircleCollider.h"
#include "Components/ComponentMetadata.h"
#include "Components/ComponentRigidbody.h"
#include "Components/ComponentScript.h"
#include "Components/ComponentSpriteRenderer.h"
#include "Components/ComponentTransform.h"
#include "SimpleAPI/BoxCollider.h"
#include "SimpleAPI/Camera.h"
#include "SimpleAPI/CircleCollider.h"
#include "SimpleAPI/Rigidbody.h"
#include "SimpleAPI/SpriteRenderer.h"

#include "entt.hpp"
#include "json.hpp"

#include <algorithm>

using namespace Vortx;
using namespace TestUtils;

class SerializerTest : public SceneTest
{
protected:
    void RoundTrip() { scene().Deserialize(scene().Serialize()); }
    static std::string Key(entt::id_type id) { return std::to_string(id); }
};

TEST_F(SerializerTest, ResetRegistersBuiltInComponents)
{
    for (auto id : { entt::type_hash<MetadataData>::value(), entt::type_hash<TransformData>::value(),
                     entt::type_hash<ScriptComponent>::value(), entt::type_hash<SpriteRendererData>::value(),
                     entt::type_hash<CameraData>::value(), entt::type_hash<BoxColliderData>::value(),
                     entt::type_hash<CircleColliderData>::value(), entt::type_hash<RigidbodyData>::value() })
    {
        EXPECT_EQ(SerializerLoader::writeFns.count(id), 1u);
        EXPECT_EQ(SerializerLoader::readFns.count(id), 1u);
    }
}

TEST_F(SerializerTest, SerializedBytesContainEntitiesWithParentLinks)
{
    GameObject p = Make("P");
    GameObject c = Make("C");
    Tr(c).SetParent(Tr(p));

    nlohmann::json root = nlohmann::json::from_cbor(SerializerLoader::SerializeScene(scene()));
    ASSERT_TRUE(root["entities"].is_array());
    ASSERT_EQ(root["entities"].size(), 2u);

    const auto& first = root["entities"][0];
    const auto& second = root["entities"][1];
    EXPECT_EQ(first["parentUUID"], nlohmann::json::array({ 0, 0 }));
    EXPECT_EQ(second["parentUUID"], first["uuid"]);
    EXPECT_TRUE(first["components"].contains(Key(entt::type_hash<MetadataData>::value())));
    EXPECT_TRUE(first["components"].contains(Key(entt::type_hash<TransformData>::value())));
}

TEST_F(SerializerTest, EmptySceneRoundTrips)
{
    RoundTrip();
    EXPECT_TRUE(scene().GetChildren().empty());
}

TEST_F(SerializerTest, DeserializeGarbageIsIgnored)
{
    Scene fresh;
    SerializerLoader::DeserializeScene(fresh, { 0xFF, 0x00, 0x13, 0x37 });
    EXPECT_TRUE(fresh.GetChildren().empty());
}

TEST_F(SerializerTest, DeserializeWithoutEntitiesArrayIsIgnored)
{
    Scene fresh;
    SerializerLoader::DeserializeScene(fresh, nlohmann::json::to_cbor(nlohmann::json{ {"x", 1} }));
    EXPECT_TRUE(fresh.GetChildren().empty());
}

TEST_F(SerializerTest, MetadataRoundTrip)
{
    GameObject go = Make("Named");
    go.TagSet("Enemy");
    RoundTrip();

    auto found = Find("Named");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->TagGet(), "Enemy");
}

TEST_F(SerializerTest, TransformRoundTrip)
{
    GameObject go = Make("T");
    Tr(go).SetPosition({ 1.5f, -2.5f });
    Tr(go).SetRotation(0.75f);
    Tr(go).SetScale({ 2, 3 });
    RoundTrip();

    Transform t = Tr(*Find("T"));
    EXPECT_VEC2_NEAR(t.Position(), glm::vec2(1.5f, -2.5f), 1e-4f);
    EXPECT_NEAR(t.Rotation(), 0.75f, 1e-4f);
    EXPECT_VEC2_NEAR(t.Scale(), glm::vec2(2, 3), 1e-4f);
}

TEST_F(SerializerTest, SpriteRendererRoundTrip)
{
    GameObject go = Make("S");
    SpriteRenderer sr = go.AddComponent<SpriteRenderer>();
    sr.Data().textureID = AssetID(5, 6);
    sr.Data().color = glm::vec4(0.1f, 0.2f, 0.3f, 0.4f);
    sr.Data().flipX = true;
    sr.Data().layer = 7;
    sr.Data().orderInLayer = -2;
    RoundTrip();

    SpriteRendererData& d = Find("S")->GetEntity().GetComponent<SpriteRendererData>();
    EXPECT_TRUE(d.textureID == AssetID(5, 6));
    EXPECT_NEAR(d.color.g, 0.2f, 1e-5f);
    EXPECT_TRUE(d.flipX);
    EXPECT_FALSE(d.flipY);
    EXPECT_EQ(d.layer, 7);
    EXPECT_EQ(d.orderInLayer, -2);
}

TEST_F(SerializerTest, BoxColliderRoundTrip)
{
    GameObject go = Make("B");
    BoxCollider b = go.AddComponent<BoxCollider>();
    b.SetOffset({ 1, 2 });
    b.SetSize({ 3, 4 });
    b.SetDensity(2.5f);
    b.SetTrigger(true);
    b.SetFriction(0.9f);
    b.SetRestitution(0.4f);
    CollisionFilter f; f.categoryBits = 4; f.maskBits = 8; f.groupIndex = -3;
    b.SetFilter(f);
    RoundTrip();

    BoxCollider r = Find("B")->GetComponent<BoxCollider>();
    EXPECT_VEC2_NEAR(r.GetOffset(), glm::vec2(1, 2), 1e-5f);
    EXPECT_VEC2_NEAR(r.GetSize(), glm::vec2(3, 4), 1e-5f);
    EXPECT_FLOAT_EQ(r.GetDensity(), 2.5f);
    EXPECT_TRUE(r.IsTrigger());
    EXPECT_NEAR(r.GetFriction(), 0.9f, 1e-5f);
    EXPECT_NEAR(r.GetRestitution(), 0.4f, 1e-5f);
    EXPECT_EQ(r.GetFilter().categoryBits, 4u);
    EXPECT_EQ(r.GetFilter().maskBits, 8u);
    EXPECT_EQ(r.GetFilter().groupIndex, -3);
}

TEST_F(SerializerTest, CircleColliderRoundTrip)
{
    GameObject go = Make("C");
    CircleCollider c = go.AddComponent<CircleCollider>();
    c.SetRadius(1.25f);
    c.SetOffset({ -1, 1 });
    RoundTrip();

    CircleCollider r = Find("C")->GetComponent<CircleCollider>();
    EXPECT_FLOAT_EQ(r.GetRadius(), 1.25f);
    EXPECT_VEC2_NEAR(r.GetOffset(), glm::vec2(-1, 1), 1e-5f);
}

TEST_F(SerializerTest, RigidbodyRoundTrip)
{
    GameObject go = Make("R");
    Rigidbody rb = go.AddComponent<Rigidbody>();
    rb.SetBodyType(BodyType::Kinematic);
    rb.SetMass(5.0f);
    rb.SetGravityScale(0.5f);
    rb.SetLinearDamping(0.1f);
    rb.SetFixedRotation(true);
    rb.SetSleepingAllowed(false);
    RoundTrip();

    Rigidbody r = Find("R")->GetComponent<Rigidbody>();
    EXPECT_EQ(r.GetBodyType(), BodyType::Kinematic);
    EXPECT_FLOAT_EQ(r.GetMass(), 5.0f);
    EXPECT_FLOAT_EQ(r.GetGravityScale(), 0.5f);
    EXPECT_NEAR(r.GetLinearDamping(), 0.1f, 1e-5f);
    EXPECT_TRUE(r.IsFixedRotation());
    EXPECT_FALSE(r.IsSleepingAllowed());
}

TEST_F(SerializerTest, ScriptNamesRoundTripWithoutInstances)
{
    GameObject go = Make("Sc");
    auto& sc = go.GetEntity().AddComponent<ScriptComponent>();
    sc.scripts.push_back({ "class Foo", nullptr });
    sc.scripts.push_back({ "class Bar", nullptr });
    RoundTrip();

    auto& r = Find("Sc")->GetEntity().GetComponent<ScriptComponent>();
    ASSERT_EQ(r.scripts.size(), 2u);
    EXPECT_EQ(r.scripts[0].name, "class Foo");
    EXPECT_EQ(r.scripts[1].name, "class Bar");
    EXPECT_EQ(r.scripts[0].instance, nullptr);
}

TEST_F(SerializerTest, CameraSizeRoundTripsForWholeNumbers)
{
    GameObject go = Make("Cam");
    Camera cam = go.AddComponent<Camera>();
    cam.SetSize(8.0f);
    RoundTrip();
    EXPECT_FLOAT_EQ(Find("Cam")->GetComponent<Camera>().GetSize(), 8.0f);
}

TEST_F(SerializerTest, CameraFractionalSizeRoundTrips)
{
    GameObject go = Make("Cam");
    Camera cam = go.AddComponent<Camera>();
    cam.SetSize(7.5f);
    RoundTrip();
    EXPECT_FLOAT_EQ(Find("Cam")->GetComponent<Camera>().GetSize(), 7.5f);
}

// ------------------------- asset dependency collection -------------------------

TEST_F(SerializerTest, CollectSceneAssetsDedupesAndSkipsNull)
{
    for (int i = 0; i < 3; i++)
    {
        SpriteRenderer sr = Make().AddComponent<SpriteRenderer>();
        sr.Data().textureID = (i == 2) ? AssetID(0) : AssetID(1, 1);
    }
    SpriteRenderer other = Make().AddComponent<SpriteRenderer>();
    other.Data().textureID = AssetID(2, 2);

    auto refs = SerializerLoader::CollectSceneAssets(scene());
    EXPECT_EQ(refs.size(), 2u);
    EXPECT_NE(std::find(refs.begin(), refs.end(), AssetID(1, 1)), refs.end());
    EXPECT_NE(std::find(refs.begin(), refs.end(), AssetID(2, 2)), refs.end());
}

TEST_F(SerializerTest, CollectSceneAssetsFromFileAndCacheInvalidation)
{
    TempDir tmp;
    auto path = (tmp / "a.scene").string();

    SpriteRenderer sr = Make().AddComponent<SpriteRenderer>();
    sr.Data().textureID = AssetID(10, 10);
    SceneManager::SaveScene(path);

    auto first = SerializerLoader::CollectSceneAssetsFromFile(path);
    ASSERT_EQ(first.size(), 1u);
    EXPECT_TRUE(first[0] == AssetID(10, 10));
    EXPECT_EQ(SerializerLoader::CollectSceneAssetsFromFile(path).size(), 1u);   // cached

    sr.Data().textureID = AssetID(20, 20);
    SceneManager::SaveScene(path);   // invalidates the cache entry

    auto second = SerializerLoader::CollectSceneAssetsFromFile(path);
    ASSERT_EQ(second.size(), 1u);
    EXPECT_TRUE(second[0] == AssetID(20, 20));
}

TEST_F(SerializerTest, CollectSceneAssetsFromMissingOrCorruptFileIsEmpty)
{
    TempDir tmp;
    EXPECT_TRUE(SerializerLoader::CollectSceneAssetsFromFile((tmp / "none.scene").string()).empty());

    WriteFile(tmp / "bad.scene", "not cbor at all");
    EXPECT_TRUE(SerializerLoader::CollectSceneAssetsFromFile((tmp / "bad.scene").string()).empty());
}
