#include "TestUtils.h"
#include "AssetPropertiesLoader.h"
#include "PropertiesLoader.h"
#include "Components/ComponentBoxCollider.h"
#include "Components/ComponentCamera.h"
#include "Components/ComponentCircleCollider.h"
#include "Components/ComponentMetadata.h"
#include "Components/ComponentRigidbody.h"
#include "Components/ComponentScript.h"
#include "Components/ComponentSpriteRenderer.h"
#include "Components/ComponentTransform.h"

using namespace Vortx;

// Only registration is covered here: the draw lambdas need a live ImGui frame.

class PropertiesLoaderTest : public SceneTest
{
protected:
    void SetUp() override { SceneTest::SetUp(); PropertiesLoader::Reset(); }

    template<typename T>
    static const PropertiesLoader::ComponentData& Data() { return PropertiesLoader::componentData.at(entt::type_hash<T>::value()); }
};

TEST_F(PropertiesLoaderTest, ResetRegistersAllBuiltInComponents)
{
    EXPECT_EQ(PropertiesLoader::componentData.size(), 8u);
    EXPECT_EQ(Data<TransformData>().name, "Transform");
    EXPECT_EQ(Data<MetadataData>().name, "Object");
    EXPECT_EQ(Data<ScriptComponent>().name, "Scripts");
    EXPECT_EQ(Data<SpriteRendererData>().name, "Sprite Renderer");
    EXPECT_EQ(Data<CameraData>().name, "Camera");
    EXPECT_EQ(Data<BoxColliderData>().name, "BoxCollider");
    EXPECT_EQ(Data<CircleColliderData>().name, "CircleCollider");
    EXPECT_EQ(Data<RigidbodyData>().name, "Rigidbody");
}

TEST_F(PropertiesLoaderTest, OnlyScriptsRenderWithoutHeader)
{
    EXPECT_FALSE(Data<ScriptComponent>().useHeader);
    EXPECT_TRUE(Data<TransformData>().useHeader);
    EXPECT_TRUE(Data<RigidbodyData>().useHeader);
}

TEST_F(PropertiesLoaderTest, EveryComponentHasDrawAndAddFunctions)
{
    for (auto& [id, data] : PropertiesLoader::componentData)
    {
        EXPECT_EQ(PropertiesLoader::componentFunc.count(id), 1u) << data.name;
        EXPECT_EQ(PropertiesLoader::componentAddFunc.count(id), 1u) << data.name;
    }
}

TEST_F(PropertiesLoaderTest, ResetIsIdempotent)
{
    PropertiesLoader::Reset();
    PropertiesLoader::Reset();
    EXPECT_EQ(PropertiesLoader::componentData.size(), 8u);
    EXPECT_EQ(PropertiesLoader::componentFunc.size(), 8u);
}

TEST_F(PropertiesLoaderTest, AddFunctionsAttachTheirComponent)
{
    GameObject go = Make();
    Entity e = go.GetEntity();

    PropertiesLoader::componentAddFunc.at(entt::type_hash<BoxColliderData>::value())(e);
    PropertiesLoader::componentAddFunc.at(entt::type_hash<CircleColliderData>::value())(e);
    PropertiesLoader::componentAddFunc.at(entt::type_hash<RigidbodyData>::value())(e);
    PropertiesLoader::componentAddFunc.at(entt::type_hash<SpriteRendererData>::value())(e);
    PropertiesLoader::componentAddFunc.at(entt::type_hash<CameraData>::value())(e);
    PropertiesLoader::componentAddFunc.at(entt::type_hash<ScriptComponent>::value())(e);

    EXPECT_TRUE(e.HasComponent<BoxColliderData>());
    EXPECT_TRUE(e.HasComponent<CircleColliderData>());
    EXPECT_TRUE(e.HasComponent<RigidbodyData>());
    EXPECT_TRUE(e.HasComponent<SpriteRendererData>());
    EXPECT_TRUE(e.HasComponent<CameraData>());
    EXPECT_TRUE(e.HasComponent<ScriptComponent>());
}

// ----------------------------- AssetPropertiesLoader -----------------------------

TEST(AssetPropertiesLoader, ResetRegistersTextureEditor)
{
    AssetPropertiesLoader::Reset();

    EXPECT_EQ(AssetPropertiesLoader::assetFunc.count(AssetType::Texture), 1u);
    ASSERT_EQ(AssetPropertiesLoader::assetData.count(AssetType::Texture), 1u);
    auto& data = AssetPropertiesLoader::assetData.at(AssetType::Texture);
    EXPECT_TRUE(data.hasApply);
}

TEST(AssetPropertiesLoader, ResetClearsPreviousRegistrations)
{
    AssetPropertiesLoader::assetFunc[AssetType::Audio] = [](const AssetID&, AssetMeta&) {};
    AssetPropertiesLoader::Reset();
    EXPECT_EQ(AssetPropertiesLoader::assetFunc.count(AssetType::Audio), 0u);
    EXPECT_EQ(AssetPropertiesLoader::assetFunc.size(), 1u);
}
