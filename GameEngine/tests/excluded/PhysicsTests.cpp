#include "TestUtils.h"
#include "Components/ComponentBoxCollider.h"
#include "Components/ComponentCircleCollider.h"
#include "Components/ComponentRigidbody.h"
#include "Physics/Physics.h"
#include "SimpleAPI/BoxCollider.h"
#include "SimpleAPI/CircleCollider.h"
#include "SimpleAPI/Rigidbody.h"

using namespace Vortx;

namespace { constexpr float kDt = 0.02f; }

class PhysicsTest : public SceneTest
{
protected:
    void SetUp() override
    {
        SceneTest::SetUp();
        Physics::Init();
        Physics::SetGravity({ 0.0f, -9.81f });
    }

    void TearDown() override
    {
        Physics::Reset();
        Physics::SetGravity({ 0.0f, -9.81f });
        SceneTest::TearDown();
    }

    // Colliders without a Rigidbody act as static geometry.
    GameObject Circle(glm::vec2 pos, bool dynamic = true, float r = 0.5f)
    {
        GameObject go = Make();
        Tr(go).SetPosition(pos);
        go.AddComponent<CircleCollider>().SetRadius(r);
        if (dynamic) go.AddComponent<Rigidbody>();
        return go;
    }

    GameObject Box(glm::vec2 pos, glm::vec2 size, bool dynamic = true)
    {
        GameObject go = Make();
        Tr(go).SetPosition(pos);
        go.AddComponent<BoxCollider>().SetSize(size);
        if (dynamic) go.AddComponent<Rigidbody>();
        return go;
    }

    static void StepN(int n) { for (int i = 0; i < n; i++) Physics::Step(kDt); }
};

// ----------------------------- settings -----------------------------

TEST_F(PhysicsTest, GravityRoundTrip)
{
    Physics::SetGravity({ 1, 2 });
    EXPECT_VEC2_NEAR(Physics::GetGravity(), glm::vec2(1, 2), 1e-6f);
}

TEST_F(PhysicsTest, StepWithoutSceneIsSafe)
{
    SceneManager::current.reset();
    Physics::Step(kDt);
    SUCCEED();
}

// ------------------------------ integration ------------------------------

TEST_F(PhysicsTest, DynamicBodyFallsUnderGravity)
{
    GameObject go = Circle({ 0, 0 });
    Rigidbody rb = go.GetComponent<Rigidbody>();

    Physics::Step(kDt);
    EXPECT_NEAR(rb.GetVelocity().y, -9.81f * kDt, 1e-4f);
    EXPECT_LT(Tr(go).Position().y, 0.0f);
}

TEST_F(PhysicsTest, GravityScaleZeroHoversInPlace)
{
    GameObject go = Circle({ 0, 0 });
    go.GetComponent<Rigidbody>().SetGravityScale(0.0f);
    StepN(10);
    EXPECT_NEAR(Tr(go).Position().y, 0.0f, 1e-5f);
}

TEST_F(PhysicsTest, VelocityMovesBodyByVelocityTimesDt)
{
    Physics::SetGravity({ 0, 0 });
    GameObject go = Circle({ 0, 0 });
    go.GetComponent<Rigidbody>().SetVelocity({ 1, 0 });

    Physics::Step(kDt);
    EXPECT_NEAR(Tr(go).Position().x, 1.0f * kDt, 1e-5f);
}

TEST_F(PhysicsTest, LinearDampingSlowsBody)
{
    Physics::SetGravity({ 0, 0 });
    GameObject go = Circle({ 0, 0 });
    Rigidbody rb = go.GetComponent<Rigidbody>();
    rb.SetVelocity({ 10, 0 });
    rb.SetLinearDamping(5.0f);
    StepN(10);
    EXPECT_LT(rb.GetVelocity().x, 10.0f);
    EXPECT_GT(rb.GetVelocity().x, 0.0f);
}

TEST_F(PhysicsTest, StaticBodyDoesNotMove)
{
    GameObject go = Circle({ 0, 0 });
    Rigidbody rb = go.GetComponent<Rigidbody>();
    rb.SetBodyType(BodyType::Static);
    rb.SetVelocity({ 5, 5 });
    StepN(5);
    EXPECT_VEC2_NEAR(Tr(go).Position(), glm::vec2(0, 0), 1e-6f);
}

TEST_F(PhysicsTest, KinematicBodyMovesByVelocityIgnoringGravity)
{
    GameObject go = Circle({ 0, 0 });
    Rigidbody rb = go.GetComponent<Rigidbody>();
    rb.SetBodyType(BodyType::Kinematic);
    rb.SetVelocity({ 1, 0 });
    StepN(10);
    EXPECT_NEAR(Tr(go).Position().x, 10 * kDt, 1e-4f);
    EXPECT_NEAR(Tr(go).Position().y, 0.0f, 1e-6f);
}

TEST_F(PhysicsTest, AppliedForceAcceleratesBodyOnce)
{
    Physics::SetGravity({ 0, 0 });
    GameObject go = Circle({ 0, 0 });
    Rigidbody rb = go.GetComponent<Rigidbody>();
    rb.ApplyForce({ 10, 0 });

    Physics::Step(kDt);
    EXPECT_NEAR(rb.GetVelocity().x, 10.0f * kDt, 1e-4f);
    EXPECT_VEC2_NEAR(rb.Data().forceAccum, glm::vec2(0, 0), 1e-6f);
}

// ------------------------------- collisions -------------------------------

TEST_F(PhysicsTest, CircleLandsOnStaticBoxAndStaysAbove)
{
    Box({ 0, -1.0f }, { 10, 1 }, false);          // top face at y = -0.5
    GameObject ball = Circle({ 0, 0.05f });

    StepN(200);

    float y = Tr(ball).Position().y;
    EXPECT_GT(y, -0.2f);
    EXPECT_LT(y, 0.3f);
}

TEST_F(PhysicsTest, BoxRestsOnStaticBox)
{
    Box({ 0, -1.0f }, { 10, 1 }, false);
    GameObject box = Box({ 0, 0.05f }, { 1, 1 });

    StepN(200);

    float y = Tr(box).Position().y;
    EXPECT_GT(y, -0.3f);
    EXPECT_LT(y, 0.3f);
}

TEST_F(PhysicsTest, OverlappingCirclesReportEnterOnce)
{
    Physics::SetGravity({ 0, 0 });
    Circle({ 0, 0 });
    Circle({ 0.8f, 0 });

    Physics::Step(kDt);
    ASSERT_EQ(Physics::GetCollisionEnters().size(), 1u);
    EXPECT_TRUE(Physics::GetCollisionStays().empty());
    EXPECT_TRUE(Physics::GetCollisionExits().empty());

    const Contact& c = Physics::GetCollisionEnters()[0];
    EXPECT_LT(c.separation, 0.0f);
    EXPECT_NEAR(std::abs(c.normal.x), 1.0f, 1e-4f);

    Physics::Step(kDt);
    EXPECT_TRUE(Physics::GetCollisionEnters().empty());   // now a stay (or exit), never enter again
}

TEST_F(PhysicsTest, SeparatedCirclesReportExit)
{
    Physics::SetGravity({ 0, 0 });
    Circle({ 0, 0 });
    GameObject b = Circle({ 0.8f, 0 });

    Physics::Step(kDt);
    ASSERT_EQ(Physics::GetCollisionEnters().size(), 1u);

    Tr(b).SetPosition({ 10, 0 });
    Physics::Step(kDt);

    EXPECT_EQ(Physics::GetCollisionExits().size(), 1u);
    EXPECT_TRUE(Physics::GetCollisionEnters().empty());
}

TEST_F(PhysicsTest, DistantCirclesDoNotCollide)
{
    Physics::SetGravity({ 0, 0 });
    Circle({ 0, 0 });
    Circle({ 5, 0 });
    Physics::Step(kDt);
    EXPECT_TRUE(Physics::GetCollisionEnters().empty());
}

TEST_F(PhysicsTest, TwoStaticCollidersNeverCollide)
{
    Circle({ 0, 0 }, false);
    Circle({ 0.1f, 0 }, false);
    Physics::Step(kDt);
    EXPECT_TRUE(Physics::GetCollisionEnters().empty());
}

TEST_F(PhysicsTest, TriggersReportTriggerEventsNotCollisions)
{
    Physics::SetGravity({ 0, 0 });
    GameObject sensor = Circle({ 0, 0 }, false);
    sensor.GetComponent<CircleCollider>().SetTrigger(true);
    GameObject body = Circle({ 0.5f, 0 });

    Physics::Step(kDt);
    EXPECT_EQ(Physics::GetTriggerEnters().size(), 1u);
    EXPECT_TRUE(Physics::GetCollisionEnters().empty());

    Physics::Step(kDt);
    EXPECT_TRUE(Physics::GetTriggerEnters().empty());
    EXPECT_EQ(Physics::GetTriggerStays().size(), 1u);

    Tr(body).SetPosition({ 20, 0 });
    Physics::Step(kDt);
    EXPECT_EQ(Physics::GetTriggerExits().size(), 1u);
}

TEST_F(PhysicsTest, NegativeSharedGroupDisablesCollision)
{
    Physics::SetGravity({ 0, 0 });
    GameObject a = Circle({ 0, 0 });
    GameObject b = Circle({ 0.8f, 0 });
    CollisionFilter f; f.groupIndex = -1;
    a.GetComponent<CircleCollider>().SetFilter(f);
    b.GetComponent<CircleCollider>().SetFilter(f);

    Physics::Step(kDt);
    EXPECT_TRUE(Physics::GetCollisionEnters().empty());
}

TEST_F(PhysicsTest, PositiveSharedGroupForcesCollisionEvenWithEmptyMasks)
{
    Physics::SetGravity({ 0, 0 });
    GameObject a = Circle({ 0, 0 });
    GameObject b = Circle({ 0.8f, 0 });
    CollisionFilter f; f.groupIndex = 2; f.maskBits = 0;
    a.GetComponent<CircleCollider>().SetFilter(f);
    b.GetComponent<CircleCollider>().SetFilter(f);

    Physics::Step(kDt);
    EXPECT_EQ(Physics::GetCollisionEnters().size(), 1u);
}

TEST_F(PhysicsTest, CategoryMaskFilteringBlocksCollision)
{
    Physics::SetGravity({ 0, 0 });
    GameObject a = Circle({ 0, 0 });
    GameObject b = Circle({ 0.8f, 0 });
    CollisionFilter fa; fa.categoryBits = 2;
    CollisionFilter fb; fb.maskBits = 1;      // b ignores category 2
    a.GetComponent<CircleCollider>().SetFilter(fa);
    b.GetComponent<CircleCollider>().SetFilter(fb);

    Physics::Step(kDt);
    EXPECT_TRUE(Physics::GetCollisionEnters().empty());
}

TEST_F(PhysicsTest, ResetClearsEventBuffersAndContactMemory)
{
    Physics::SetGravity({ 0, 0 });
    Circle({ 0, 0 });
    Circle({ 0.8f, 0 });
    Physics::Step(kDt);
    ASSERT_FALSE(Physics::GetCollisionEnters().empty());

    Physics::Reset();
    EXPECT_TRUE(Physics::GetCollisionEnters().empty());

    Physics::Step(kDt);
    EXPECT_EQ(Physics::GetCollisionEnters().size(), 1u);   // reported as a fresh enter again
}

// -------------------------------- sleeping --------------------------------

TEST_F(PhysicsTest, IdleBodyFallsAsleepAndCountDrops)
{
    Physics::SetGravity({ 0, 0 });
    GameObject go = Circle({ 0, 0 });
    Rigidbody rb = go.GetComponent<Rigidbody>();

    EXPECT_EQ(Physics::GetActiveBodyCount(), 1u);
    StepN(40);   // 0.8 s > 0.5 s sleep delay

    EXPECT_FALSE(rb.IsAwake());
    EXPECT_EQ(Physics::GetActiveBodyCount(), 0u);

    rb.ApplyImpulse({ 1, 0 });
    EXPECT_TRUE(rb.IsAwake());
}

TEST_F(PhysicsTest, BodyThatDisallowsSleepStaysAwake)
{
    Physics::SetGravity({ 0, 0 });
    GameObject go = Circle({ 0, 0 });
    Rigidbody rb = go.GetComponent<Rigidbody>();
    rb.SetSleepingAllowed(false);
    StepN(60);
    EXPECT_TRUE(rb.IsAwake());
}

// ---------------------------------- mass ----------------------------------

TEST_F(PhysicsTest, AutoMassFromBoxCollider)
{
    GameObject go = Make();
    BoxCollider b = go.AddComponent<BoxCollider>();
    b.SetSize({ 2, 3 });
    b.SetDensity(2.0f);
    Rigidbody rb = go.AddComponent<Rigidbody>();
    rb.SetAutoMass(true);

    Physics::RecomputeMassNow(go.GetEntity());

    EXPECT_NEAR(rb.GetMass(), 12.0f, 1e-4f);
    EXPECT_NEAR(rb.GetInverseMass(), 1.0f / 12.0f, 1e-5f);
    EXPECT_NEAR(rb.GetInertia(), 13.0f, 1e-3f);   // m (w^2 + h^2) / 12
    EXPECT_FALSE(rb.Data().massDirty);
}

TEST_F(PhysicsTest, AutoMassFromCircleCollider)
{
    GameObject go = Make();
    CircleCollider c = go.AddComponent<CircleCollider>();
    c.SetRadius(1.0f);
    Rigidbody rb = go.AddComponent<Rigidbody>();
    rb.SetAutoMass(true);

    Physics::RecomputeMassNow(go.GetEntity());

    EXPECT_NEAR(rb.GetMass(), 3.14159265f, 1e-4f);
    EXPECT_NEAR(rb.GetInertia(), 0.5f * 3.14159265f, 1e-4f);
}

TEST_F(PhysicsTest, AutoMassWithoutCollidersFallsBackToUnit)
{
    GameObject go = Make();
    Rigidbody rb = go.AddComponent<Rigidbody>();
    rb.SetAutoMass(true);
    Physics::RecomputeMassNow(go.GetEntity());
    EXPECT_FLOAT_EQ(rb.GetMass(), 1.0f);
    EXPECT_FLOAT_EQ(rb.GetInertia(), 1.0f);
}

TEST_F(PhysicsTest, ManualMassIsNotOverwritten)
{
    GameObject go = Make();
    go.AddComponent<BoxCollider>().SetSize({ 10, 10 });
    Rigidbody rb = go.AddComponent<Rigidbody>();
    rb.SetMass(3.0f);
    Physics::RecomputeMassNow(go.GetEntity());
    EXPECT_FLOAT_EQ(rb.GetMass(), 3.0f);
}

TEST_F(PhysicsTest, StaticAndFixedRotationBodiesHaveZeroInverses)
{
    GameObject go = Make();
    go.AddComponent<BoxCollider>();
    Rigidbody rb = go.AddComponent<Rigidbody>();
    rb.SetAutoMass(true);

    rb.SetFixedRotation(true);
    Physics::RecomputeMassNow(go.GetEntity());
    EXPECT_GT(rb.GetInverseMass(), 0.0f);
    EXPECT_FLOAT_EQ(rb.Data().inverseInertia, 0.0f);

    rb.SetBodyType(BodyType::Static);
    Physics::RecomputeMassNow(go.GetEntity());
    EXPECT_FLOAT_EQ(rb.GetInverseMass(), 0.0f);
}

TEST_F(PhysicsTest, MarkMassDirtyIsConsumedByNextStep)
{
    GameObject go = Make();
    go.AddComponent<BoxCollider>().SetSize({ 2, 2 });
    Rigidbody rb = go.AddComponent<Rigidbody>();
    rb.SetAutoMass(true);
    Physics::SetGravity({ 0, 0 });

    Physics::MarkMassDirty(go.GetEntity());
    EXPECT_TRUE(rb.Data().massDirty);
    Physics::Step(kDt);
    EXPECT_FALSE(rb.Data().massDirty);
    EXPECT_NEAR(rb.GetMass(), 4.0f, 1e-4f);
}

TEST_F(PhysicsTest, MarkMassDirtyOnEntityWithoutBodyIsNoOp)
{
    Physics::MarkMassDirty(Make().GetEntity());
    SUCCEED();
}

// ------------------------- queries (not implemented yet) -------------------------

TEST_F(PhysicsTest, QueryStubsReturnNothing)
{
    Circle({ 0, 0 });
    RaycastHit hit;
    Entity e;
    EXPECT_FALSE(Physics::Raycast({ -5, 0 }, { 1, 0 }, 10, hit));
    EXPECT_TRUE(Physics::RaycastAll({ -5, 0 }, { 1, 0 }, 10).empty());
    EXPECT_FALSE(Physics::OverlapPoint({ 0, 0 }, e));
    EXPECT_TRUE(Physics::OverlapCircle({ 0, 0 }, 1).empty());
    EXPECT_TRUE(Physics::OverlapBox({ 0, 0 }, { 1, 1 }, glm::vec1(0)).empty());
}
