#include "TestUtils.h"
#include "SimpleAPI/Math.h"
#include "SimpleAPI/Time.h"
#include "SimpleAPI/Gizmos.h"

using namespace Vortx;

// ------------------------------- Math -------------------------------

TEST(Math, CrossVecVec)
{
    EXPECT_FLOAT_EQ(Math::Cross({ 1, 0 }, { 0, 1 }), 1.0f);
    EXPECT_FLOAT_EQ(Math::Cross({ 0, 1 }, { 1, 0 }), -1.0f);
    EXPECT_FLOAT_EQ(Math::Cross({ 2, 3 }, { 2, 3 }), 0.0f);
}

TEST(Math, CrossScalarVec)
{
    EXPECT_VEC2_NEAR(Math::Cross(2.0f, glm::vec2(1, 0)), glm::vec2(0, 2), 1e-6f);
    EXPECT_VEC2_NEAR(Math::Cross(2.0f, glm::vec2(0, 1)), glm::vec2(-2, 0), 1e-6f);
}

TEST(Math, RotateQuarterTurn)
{
    EXPECT_VEC2_NEAR(Math::Rotate({ 1, 0 }, Math::Pi / 2), glm::vec2(0, 1), 1e-5f);
    EXPECT_VEC2_NEAR(Math::Rotate({ 0, 1 }, Math::Pi / 2), glm::vec2(-1, 0), 1e-5f);
}

TEST(Math, RotateZeroIsIdentityAndInverseCancels)
{
    glm::vec2 v(3, -4);
    EXPECT_VEC2_NEAR(Math::Rotate(v, 0), v, 1e-6f);
    EXPECT_VEC2_NEAR(Math::Rotate(Math::Rotate(v, 0.7f), -0.7f), v, 1e-5f);
}

TEST(Math, Distance)
{
    EXPECT_FLOAT_EQ(Math::Distance({ 0, 0 }, { 3, 4 }), 5.0f);
}

TEST(Math, AngleConstants)
{
    EXPECT_NEAR(Math::Pi, 3.14159265f, 1e-6f);
    EXPECT_NEAR(180.0f * Math::DegToRad, Math::Pi, 1e-5f);
    EXPECT_NEAR(Math::Pi * Math::RadToDeg, 180.0f, 1e-3f);
}

// ------------------------------- Time -------------------------------

class TimeTest : public ::testing::Test
{
protected:
    void TearDown() override { Time t; t.SetTimeScale(1.0f); }
};

TEST_F(TimeTest, TimeScaleRoundTrip)
{
    Time t;
    t.SetTimeScale(2.5f);
    EXPECT_FLOAT_EQ(t.GetTimeScale(), 2.5f);
}

TEST_F(TimeTest, SetDeltaTimeAppliesScale)
{
    Time t;
    t.SetTimeScale(0.5f);
    Time::_SetDeltaTime(0.1f);
    EXPECT_FLOAT_EQ(Time::unscaledDeltaTime, 0.1f);
    EXPECT_FLOAT_EQ(Time::deltaTime, 0.05f);
}

TEST_F(TimeTest, NegativeScaleClampsToZeroWhenApplied)
{
    Time t;
    t.SetTimeScale(-3.0f);
    Time::_SetDeltaTime(0.1f);
    EXPECT_FLOAT_EQ(Time::deltaTime, 0.0f);
    EXPECT_FLOAT_EQ(t.GetTimeScale(), 0.0f);
}

TEST_F(TimeTest, FixedDeltaDefaults)
{
    EXPECT_FLOAT_EQ(Time::unscaledFixedDeltaTime, 0.02f);
}

// ------------------------------ Gizmos ------------------------------

class GizmosTest : public ::testing::Test
{
protected:
    void SetUp() override { Gizmos::enabled = true; Gizmos::color = glm::u8vec4(255); Gizmos::Clear(); }
    void TearDown() override { Gizmos::Clear(); Gizmos::enabled = false; Gizmos::color = glm::u8vec4(255); }
};

TEST_F(GizmosTest, DisabledRecordsNothing)
{
    Gizmos::enabled = false;
    Gizmos::DrawCircle({ 0, 0 }, 1);
    Gizmos::DrawBox({ 0, 0 }, { 1, 1 });
    Gizmos::DrawLine({ 0, 0 }, { 1, 1 });
    Gizmos::DrawRay({ 0, 0 }, { 1, 1 });
    EXPECT_TRUE(Gizmos::commands.empty());
}

TEST_F(GizmosTest, CircleCommand)
{
    Gizmos::DrawCircle({ 1, 2 }, 3, 4);
    ASSERT_EQ(Gizmos::commands.size(), 1u);
    auto& c = Gizmos::commands[0];
    EXPECT_EQ(c.type, Gizmos::Circle);
    EXPECT_FLOAT_EQ(c.data[0], 1); EXPECT_FLOAT_EQ(c.data[1], 2);
    EXPECT_FLOAT_EQ(c.data[2], 3); EXPECT_FLOAT_EQ(c.data[3], 4);
}

TEST_F(GizmosTest, BoxCommandLayout)
{
    Gizmos::DrawBox({ 1, 2 }, { 3, 4 }, 0.5f, 2.0f);
    ASSERT_EQ(Gizmos::commands.size(), 1u);
    auto& c = Gizmos::commands[0];
    EXPECT_EQ(c.type, Gizmos::Box);
    EXPECT_FLOAT_EQ(c.data[2], 3); EXPECT_FLOAT_EQ(c.data[3], 4);
    EXPECT_FLOAT_EQ(c.data[4], 2.0f);   // thickness
    EXPECT_FLOAT_EQ(c.data[5], 0.5f);   // rotation
}

TEST_F(GizmosTest, LineAndRay)
{
    Gizmos::DrawLine({ 0, 0 }, { 1, 1 }, 3);
    Gizmos::DrawRay({ 1, 1 }, { 2, 0 });
    ASSERT_EQ(Gizmos::commands.size(), 2u);
    EXPECT_EQ(Gizmos::commands[0].type, Gizmos::Line);
    EXPECT_FLOAT_EQ(Gizmos::commands[0].data[4], 3);
    EXPECT_FLOAT_EQ(Gizmos::commands[1].data[2], 3);   // ray end x = 1 + 2
    EXPECT_FLOAT_EQ(Gizmos::commands[1].data[3], 1);
}

TEST_F(GizmosTest, CommandsCaptureCurrentColor)
{
    Gizmos::color = glm::u8vec4(10, 20, 30, 40);
    Gizmos::DrawLine({ 0, 0 }, { 1, 0 });
    Gizmos::color = glm::u8vec4(1, 2, 3, 4);
    Gizmos::DrawLine({ 0, 0 }, { 1, 0 });
    EXPECT_EQ(Gizmos::commands[0].color, glm::u8vec4(10, 20, 30, 40));
    EXPECT_EQ(Gizmos::commands[1].color, glm::u8vec4(1, 2, 3, 4));
}

TEST_F(GizmosTest, ClearEmptiesCommands)
{
    Gizmos::DrawCircle({ 0, 0 }, 1);
    Gizmos::Clear();
    EXPECT_TRUE(Gizmos::commands.empty());
}
