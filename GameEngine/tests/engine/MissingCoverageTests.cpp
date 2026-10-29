#include "TestUtils.h"
#include "Components/ComponentCamera.h"
#include "Engine.h"
#include "Scripting/ScriptBase.h"
#include "SimpleAPI/BoxCollider.h"
#include "SimpleAPI/Camera.h"
#include "SimpleAPI/CircleCollider.h"
#include "SimpleAPI/InputManager.h"

using namespace Vortx;

// ------------------------------ Engine ------------------------------

TEST(Engine, InitCreatesAnEmptyCurrentScene)
{
    SceneManager::current.reset();
    Engine e;
    e.Init();
    ASSERT_NE(SceneManager::current, nullptr);
    EXPECT_TRUE(SceneManager::current->GetChildren().empty());
    SceneManager::current.reset();
}

TEST(Engine, GetAndDestroyEngineExports)
{
    Engine* e = GetEngine();
    ASSERT_NE(e, nullptr);
    DestroyEngine(e);
}

// --------------------------- REGISTER_SCRIPT macro ---------------------------

class MacroScript : public ScriptBase<MacroScript> {};
REGISTER_SCRIPT(MacroScript)

TEST(ScriptMacro, RegisterScriptRunsAtStaticInit)
{
    EXPECT_TRUE(MacroScript_registered);
}

// ------------------------ BaseComponent templates ------------------------

class BaseComponentTest : public SceneTest {};

TEST_F(BaseComponentTest, HasComponentReflectsAddedWrappers)
{
    GameObject go = Make();
    EXPECT_FALSE(go.HasComponent<BoxCollider>());
    go.AddComponent<BoxCollider>();
    EXPECT_TRUE(go.HasComponent<BoxCollider>());
    EXPECT_TRUE(go.HasComponent<Transform>());
}

TEST_F(BaseComponentTest, HasComponentWorksForRawDataTypes)
{
    GameObject go = Make();
    EXPECT_FALSE(go.HasComponent<CameraData>());
    go.AddComponent<Camera>();
    EXPECT_TRUE(go.HasComponent<CameraData>());
}

TEST_F(BaseComponentTest, GetEntityReturnsOwner)
{
    GameObject go = Make();
    EXPECT_TRUE(go.GetComponent<Transform>().GetEntity() == go.GetEntity());
}

// --------------------- CircleCollider material / filter ---------------------

TEST_F(BaseComponentTest, CircleColliderMaterialAndFilter)
{
    CircleCollider c = Make().AddComponent<CircleCollider>();
    EXPECT_FLOAT_EQ(c.GetFriction(), 0.3f);
    c.SetFriction(0.7f);
    c.SetRestitution(0.6f);
    CollisionFilter f; f.categoryBits = 8; f.maskBits = 3; f.groupIndex = 4;
    c.SetFilter(f);

    EXPECT_FLOAT_EQ(c.GetFriction(), 0.7f);
    EXPECT_FLOAT_EQ(c.GetRestitution(), 0.6f);
    EXPECT_EQ(c.GetFilter().categoryBits, 8u);
    EXPECT_EQ(c.GetFilter().maskBits, 3u);
    EXPECT_EQ(c.GetFilter().groupIndex, 4);
}

TEST_F(BaseComponentTest, RegistryStorageEnumeratesComponentPools)
{
    GameObject go = Make();
    go.AddComponent<BoxCollider>();
    int pools = 0;
    for (auto&& [id, storage] : go.GetEntity().GetRegistry().Storage())
        if (storage.contains(go.GetEntity().GetValue())) pools++;
    EXPECT_GE(pools, 3);   // Metadata + Transform + BoxCollider
}

// ----------------- InputManager::GetMouseWorldPosition with a camera -----------------

class MouseWorldTest : public ::testing::Test
{
protected:
    static Scene* suiteScene;
    static void SetUpTestSuite() { SerializerLoader::Reset(); suiteScene = new Scene(); }   // leaked: Camera::main points into it
    void SetUp() override { SceneManager::current.reset(suiteScene); }
    void TearDown() override
    {
        InputManager::_SetViewportOverride(false, { 0, 0 }, { 0, 0 });
        std::vector<GameObject> all;
        for (auto& kv : suiteScene->gameObjectsRegister) all.push_back(kv.second);
        for (auto& g : all) suiteScene->DestroyGameObject(g);
        (void)SceneManager::current.release();
    }
};
Scene* MouseWorldTest::suiteScene = nullptr;

TEST_F(MouseWorldTest, NoCameraGivesOrigin)
{
    InputManager::_SetViewportOverride(true, { 0, 0 }, { 100, 100 });
    EXPECT_VEC2_NEAR(InputManager::GetMouseWorldPosition(), glm::vec2(0, 0), 1e-6f);
}

TEST_F(MouseWorldTest, TopLeftOfViewportMapsToCameraCorner)
{
    GameObject go = suiteScene->CreateGameObject();
    Camera cam = go.AddComponent<Camera>();       // size 5
    InputManager::_SetViewportOverride(true, { 0, 0 }, { 100, 100 });

    // mouse position is (0,0) in tests => top-left of the viewport
    float aspectX = 5.0f * (cam.GetProjectionMatrix()[1][1] / cam.GetProjectionMatrix()[0][0]);
    EXPECT_VEC2_NEAR(InputManager::GetMouseWorldPosition(), glm::vec2(-aspectX, 5.0f), 1e-3f);

    go.GetComponent<Transform>().SetPosition({ 10, 0 });
    EXPECT_VEC2_NEAR(InputManager::GetMouseWorldPosition(), glm::vec2(10 - aspectX, 5.0f), 1e-3f);
}
