#include "TestUtils.h"
#include "Components/ComponentBoxCollider.h"
#include "Components/ComponentCamera.h"
#include "Components/ComponentCircleCollider.h"
#include "Components/ComponentRigidbody.h"
#include "Components/ComponentSpriteRenderer.h"
#include "Renderer/GameViewport.h"
#include "SimpleAPI/BoxCollider.h"
#include "SimpleAPI/Camera.h"
#include "SimpleAPI/CircleCollider.h"
#include "SimpleAPI/Math.h"
#include "SimpleAPI/Rigidbody.h"
#include "SimpleAPI/SpriteRenderer.h"

using namespace Vortx;

class ComponentTest : public SceneTest {};

// ------------------------------ BoxCollider ------------------------------

TEST_F(ComponentTest, BoxColliderDefaults)
{
    BoxCollider b = Make().AddComponent<BoxCollider>();
    EXPECT_TRUE(b.Exists());
    EXPECT_VEC2_NEAR(b.GetOffset(), glm::vec2(0, 0), 1e-6f);
    EXPECT_VEC2_NEAR(b.GetSize(), glm::vec2(1, 1), 1e-6f);
    EXPECT_FLOAT_EQ(b.GetDensity(), 1.0f);
    EXPECT_FALSE(b.IsTrigger());
    EXPECT_FLOAT_EQ(b.GetFriction(), 0.3f);
    EXPECT_FLOAT_EQ(b.GetRestitution(), 0.0f);
    EXPECT_EQ(b.GetFilter().categoryBits, 1u);
    EXPECT_EQ(b.GetFilter().maskBits, 0xFFFFFFFFu);
}

TEST_F(ComponentTest, BoxColliderExistsIsFalseWhenMissing)
{
    EXPECT_FALSE(BoxCollider(Make().GetEntity()).Exists());
}

TEST_F(ComponentTest, BoxColliderSetters)
{
    BoxCollider b = Make().AddComponent<BoxCollider>();
    b.SetOffset({ 1, 2 });
    b.SetSize({ 3, 4 });
    b.SetTrigger(true);
    b.SetFriction(0.8f);
    b.SetRestitution(0.2f);
    EXPECT_VEC2_NEAR(b.GetOffset(), glm::vec2(1, 2), 1e-6f);
    EXPECT_VEC2_NEAR(b.GetSize(), glm::vec2(3, 4), 1e-6f);
    EXPECT_TRUE(b.IsTrigger());
    EXPECT_FLOAT_EQ(b.GetFriction(), 0.8f);
    EXPECT_FLOAT_EQ(b.GetRestitution(), 0.2f);
}

TEST_F(ComponentTest, BoxColliderClosestPoint)
{
    GameObject go = Make();
    BoxCollider b = go.AddComponent<BoxCollider>();
    b.SetSize({ 2, 2 });

    EXPECT_VEC2_NEAR(b.ClosestPoint({ 5, 0 }), glm::vec2(1, 0), 1e-5f);
    EXPECT_VEC2_NEAR(b.ClosestPoint({ 5, 5 }), glm::vec2(1, 1), 1e-5f);
    EXPECT_VEC2_NEAR(b.ClosestPoint({ 0.25f, -0.5f }), glm::vec2(0.25f, -0.5f), 1e-5f);   // inside
}

TEST_F(ComponentTest, BoxColliderClosestPointRespectsRotation)
{
    GameObject go = Make();
    Tr(go).SetRotation(Math::Pi / 2);
    BoxCollider b = go.AddComponent<BoxCollider>();
    b.SetSize({ 4, 2 });   // after a 90deg turn it is 2 wide, 4 tall

    EXPECT_VEC2_NEAR(b.ClosestPoint({ 0, 10 }), glm::vec2(0, 2), 1e-4f);
}

TEST_F(ComponentTest, BoxColliderWorldBoundsScaleAndRotation)
{
    GameObject go = Make();
    Tr(go).SetPosition({ 1, 1 });
    Tr(go).SetScale({ 2, 2 });
    BoxCollider b = go.AddComponent<BoxCollider>();

    glm::vec2 mn, mx;
    b.GetWorldBounds(mn, mx);
    EXPECT_VEC2_NEAR(mn, glm::vec2(0, 0), 1e-5f);
    EXPECT_VEC2_NEAR(mx, glm::vec2(2, 2), 1e-5f);

    Tr(go).SetScale({ 1, 1 });
    Tr(go).SetPosition({ 0, 0 });
    Tr(go).SetRotation(Math::Pi / 2);
    b.SetSize({ 2, 1 });
    b.GetWorldBounds(mn, mx);
    EXPECT_VEC2_NEAR(mx, glm::vec2(0.5f, 1.0f), 1e-4f);
    EXPECT_VEC2_NEAR(mn, glm::vec2(-0.5f, -1.0f), 1e-4f);
}

TEST_F(ComponentTest, BoxColliderDensityMarksRigidbodyMassDirtyOnlyWithAutoMass)
{
    GameObject go = Make();
    Rigidbody rb = go.AddComponent<Rigidbody>();
    BoxCollider b = go.AddComponent<BoxCollider>();

    rb.Data().massDirty = false;
    b.SetDensity(3.0f);
    EXPECT_FALSE(rb.Data().massDirty);

    rb.SetAutoMass(true);
    b.SetDensity(4.0f);
    EXPECT_TRUE(rb.Data().massDirty);
}

TEST_F(ComponentTest, BoxColliderGetGameObject)
{
    GameObject go = Make("owner");
    BoxCollider b = go.AddComponent<BoxCollider>();
    EXPECT_EQ(b.GetGameObject().NameGet(), "owner");
}

// ----------------------------- CircleCollider -----------------------------

TEST_F(ComponentTest, CircleColliderDefaultsAndSetters)
{
    CircleCollider c = Make().AddComponent<CircleCollider>();
    EXPECT_FLOAT_EQ(c.GetRadius(), 0.5f);
    c.SetRadius(2.0f);
    c.SetOffset({ 1, 1 });
    c.SetTrigger(true);
    EXPECT_FLOAT_EQ(c.GetRadius(), 2.0f);
    EXPECT_VEC2_NEAR(c.GetOffset(), glm::vec2(1, 1), 1e-6f);
    EXPECT_TRUE(c.IsTrigger());
}

TEST_F(ComponentTest, CircleColliderClosestPoint)
{
    GameObject go = Make();
    CircleCollider c = go.AddComponent<CircleCollider>();
    c.SetRadius(1.0f);

    EXPECT_VEC2_NEAR(c.ClosestPoint({ 5, 0 }), glm::vec2(1, 0), 1e-5f);
    EXPECT_VEC2_NEAR(c.ClosestPoint({ 0.3f, 0.2f }), glm::vec2(0.3f, 0.2f), 1e-6f);
}

TEST_F(ComponentTest, CircleColliderWorldBounds)
{
    GameObject go = Make();
    Tr(go).SetPosition({ 2, 3 });
    CircleCollider c = go.AddComponent<CircleCollider>();
    c.SetRadius(1.5f);

    glm::vec2 mn, mx;
    c.GetWorldBounds(mn, mx);
    EXPECT_VEC2_NEAR(mn, glm::vec2(0.5f, 1.5f), 1e-5f);
    EXPECT_VEC2_NEAR(mx, glm::vec2(3.5f, 4.5f), 1e-5f);
}

TEST_F(ComponentTest, CircleColliderDensityMarksMassDirty)
{
    GameObject go = Make();
    Rigidbody rb = go.AddComponent<Rigidbody>();
    CircleCollider c = go.AddComponent<CircleCollider>();
    rb.SetAutoMass(true);
    rb.Data().massDirty = false;
    c.SetDensity(2.0f);
    EXPECT_TRUE(rb.Data().massDirty);
}

// ------------------------------- Rigidbody -------------------------------

TEST_F(ComponentTest, RigidbodyDefaults)
{
    Rigidbody rb = Make().AddComponent<Rigidbody>();
    EXPECT_EQ(rb.GetBodyType(), BodyType::Dynamic);
    EXPECT_FLOAT_EQ(rb.GetMass(), 1.0f);
    EXPECT_FALSE(rb.GetAutoMass());
    EXPECT_TRUE(rb.IsAwake());
    EXPECT_TRUE(rb.IsSleepingAllowed());
    EXPECT_FLOAT_EQ(rb.GetGravityScale(), 1.0f);
}

TEST_F(ComponentTest, RigidbodySetMassDisablesAutoMassAndClearsDirty)
{
    Rigidbody rb = Make().AddComponent<Rigidbody>();
    rb.SetAutoMass(true);
    rb.SetMass(7.0f);
    EXPECT_FLOAT_EQ(rb.GetMass(), 7.0f);
    EXPECT_FALSE(rb.GetAutoMass());
    EXPECT_FALSE(rb.Data().massDirty);
}

TEST_F(ComponentTest, RigidbodySetBodyTypeMarksMassDirty)
{
    Rigidbody rb = Make().AddComponent<Rigidbody>();
    rb.Data().massDirty = false;
    rb.SetBodyType(BodyType::Static);
    EXPECT_EQ(rb.GetBodyType(), BodyType::Static);
    EXPECT_TRUE(rb.Data().massDirty);
}

TEST_F(ComponentTest, RigidbodyVelocityAndDampingAccessors)
{
    Rigidbody rb = Make().AddComponent<Rigidbody>();
    rb.SetVelocity({ 1, 2 });
    rb.SetAngularVelocity(3.0f);
    rb.SetLinearDamping(0.5f);
    rb.SetAngularDamping(0.25f);
    EXPECT_VEC2_NEAR(rb.GetVelocity(), glm::vec2(1, 2), 1e-6f);
    EXPECT_FLOAT_EQ(rb.GetAngularVelocity(), 3.0f);
    EXPECT_FLOAT_EQ(rb.GetLinearDamping(), 0.5f);
    EXPECT_FLOAT_EQ(rb.GetAngularDamping(), 0.25f);
}

TEST_F(ComponentTest, RigidbodyForcesAccumulateOnlyOnDynamicBodies)
{
    GameObject go = Make();
    Rigidbody rb = go.AddComponent<Rigidbody>();
    rb.ApplyForce({ 1, 0 });
    rb.ApplyForce({ 0, 2 });
    rb.ApplyTorque(3.0f);
    EXPECT_VEC2_NEAR(rb.Data().forceAccum, glm::vec2(1, 2), 1e-6f);
    EXPECT_FLOAT_EQ(rb.Data().torqueAccum, 3.0f);

    rb.SetBodyType(BodyType::Static);
    rb.Data().forceAccum = glm::vec2(0);
    rb.ApplyForce({ 5, 5 });
    EXPECT_VEC2_NEAR(rb.Data().forceAccum, glm::vec2(0, 0), 1e-6f);
}

TEST_F(ComponentTest, RigidbodyForceAtPointCreatesTorque)
{
    Rigidbody rb = Make().AddComponent<Rigidbody>();
    rb.ApplyForceAtPoint({ 0, 1 }, { 1, 0 });
    EXPECT_FLOAT_EQ(rb.Data().torqueAccum, 1.0f);
}

TEST_F(ComponentTest, RigidbodyImpulseChangesVelocityByInverseMass)
{
    Rigidbody rb = Make().AddComponent<Rigidbody>();
    rb.Data().inverseMass = 0.5f;
    rb.ApplyImpulse({ 4, 0 });
    EXPECT_VEC2_NEAR(rb.GetVelocity(), glm::vec2(2, 0), 1e-6f);
}

TEST_F(ComponentTest, RigidbodyAngularImpulseAndFixedRotation)
{
    Rigidbody rb = Make().AddComponent<Rigidbody>();
    rb.Data().inverseInertia = 2.0f;
    rb.ApplyAngularImpulse(1.5f);
    EXPECT_FLOAT_EQ(rb.GetAngularVelocity(), 3.0f);

    rb.SetFixedRotation(true);
    rb.ApplyAngularImpulse(10.0f);
    rb.ApplyTorque(10.0f);
    EXPECT_FLOAT_EQ(rb.GetAngularVelocity(), 3.0f);
    EXPECT_FLOAT_EQ(rb.Data().torqueAccum, 0.0f);
}

TEST_F(ComponentTest, RigidbodySleepAndWake)
{
    Rigidbody rb = Make().AddComponent<Rigidbody>();
    rb.SetVelocity({ 1, 1 });
    rb.Sleep();
    EXPECT_FALSE(rb.IsAwake());
    EXPECT_VEC2_NEAR(rb.GetVelocity(), glm::vec2(0, 0), 1e-6f);

    rb.ApplyImpulse({ 1, 0 });
    EXPECT_TRUE(rb.IsAwake());
}

TEST_F(ComponentTest, RigidbodySleepRespectsAllowSleepAndBodyType)
{
    Rigidbody rb = Make().AddComponent<Rigidbody>();
    rb.SetSleepingAllowed(false);
    rb.Sleep();
    EXPECT_TRUE(rb.IsAwake());

    rb.SetSleepingAllowed(true);
    rb.SetBodyType(BodyType::Kinematic);
    rb.Sleep();
    EXPECT_TRUE(rb.IsAwake());
}

TEST_F(ComponentTest, RigidbodyDisablingSleepWakesBody)
{
    Rigidbody rb = Make().AddComponent<Rigidbody>();
    rb.Sleep();
    rb.SetSleepingAllowed(false);
    EXPECT_TRUE(rb.IsAwake());
}

TEST_F(ComponentTest, RigidbodyWorldCenterOfMassAndPointVelocity)
{
    GameObject go = Make();
    Tr(go).SetPosition({ 2, 0 });
    Rigidbody rb = go.AddComponent<Rigidbody>();
    rb.Data().centerOfMass = { 1, 0 };
    EXPECT_VEC2_NEAR(rb.GetWorldCenterOfMass(), glm::vec2(3, 0), 1e-5f);
    EXPECT_VEC2_NEAR(rb.GetLocalCenterOfMass(), glm::vec2(1, 0), 1e-6f);

    rb.Data().centerOfMass = { 0, 0 };
    rb.SetVelocity({ 1, 0 });
    rb.SetAngularVelocity(2.0f);
    // r = (1,0) from COM -> omega x r = (0, 2)
    EXPECT_VEC2_NEAR(rb.GetPointVelocity({ 3, 0 }), glm::vec2(1, 2), 1e-5f);
}

// ----------------------------- SpriteRenderer -----------------------------

TEST_F(ComponentTest, SpriteRendererDefaultsAndTexture)
{
    SpriteRenderer sr = Make().AddComponent<SpriteRenderer>();
    EXPECT_TRUE(sr.Exists());
    EXPECT_TRUE(sr.GetTexture() == AssetID(0));
    EXPECT_FLOAT_EQ(sr.Data().color.a, 1.0f);

    sr.SetTexture(AssetID(3, 4));
    EXPECT_TRUE(sr.GetTexture() == AssetID(3, 4));
}

// ------------------------------- Camera -------------------------------
// Camera::main is a process-wide static pointing at a registry. To avoid it
// dangling between tests, these tests share one intentionally-leaked scene.

class CameraTest : public ::testing::Test
{
protected:
    static Scene* suiteScene;

    static void SetUpTestSuite()
    {
        SerializerLoader::Reset();
        suiteScene = new Scene();   // never freed on purpose
    }

    void SetUp() override { SceneManager::current.reset(suiteScene); }

    void TearDown() override
    {
        std::vector<GameObject> all;
        for (auto& kv : suiteScene->gameObjectsRegister) all.push_back(kv.second);
        for (auto& g : all) suiteScene->DestroyGameObject(g);
        (void)SceneManager::current.release();
    }

    GameObject Make() { return suiteScene->CreateGameObject(); }
};
Scene* CameraTest::suiteScene = nullptr;

TEST_F(CameraTest, NoCameraMeansMainIsDefault)
{
    EXPECT_TRUE(Camera::Main() == Camera());
}

TEST_F(CameraTest, FirstCameraBecomesMain)
{
    Camera cam = Make().AddComponent<Camera>();
    EXPECT_TRUE(Camera::Main() == cam);
}

TEST_F(CameraTest, ChangeMainSwitchesCamera)
{
    Camera a = Make().AddComponent<Camera>();
    ASSERT_TRUE(Camera::Main() == a);
    Camera b = Make().AddComponent<Camera>();
    EXPECT_TRUE(Camera::Main() == a);

    Camera::ChangeMain(b);
    EXPECT_TRUE(Camera::Main() == b);
}

TEST_F(CameraTest, MainFallsBackWhenMainCameraDestroyed)
{
    GameObject goA = Make();
    Camera a = goA.AddComponent<Camera>();
    ASSERT_TRUE(Camera::Main() == a);
    Camera b = Make().AddComponent<Camera>();

    suiteScene->DestroyGameObject(goA);
    EXPECT_TRUE(Camera::Main() == b);
}

TEST_F(CameraTest, SizeAndProjection)
{
    Camera cam = Make().AddComponent<Camera>();
    EXPECT_FLOAT_EQ(cam.GetSize(), 5.0f);

    cam.SetSize(10.0f);
    EXPECT_FLOAT_EQ(cam.GetSize(), 10.0f);

    glm::mat4 p = cam.GetProjectionMatrix();
    EXPECT_NEAR(p[1][1], 1.0f / 10.0f, 1e-6f);
    EXPECT_NEAR(p[0][0], 1.0f / (10.0f * GameViewport::GetAspect()), 1e-6f);
}

TEST_F(CameraTest, ProjectionViewCentersOnCameraPosition)
{
    GameObject go = Make();
    Camera cam = go.AddComponent<Camera>();
    go.GetComponent<Transform>().SetPosition({ 2, 3 });

    glm::vec4 ndc = cam.GetProjectionViewMatrix() * glm::vec4(2, 3, 0, 1);
    EXPECT_NEAR(ndc.x, 0.0f, 1e-5f);
    EXPECT_NEAR(ndc.y, 0.0f, 1e-5f);
}
