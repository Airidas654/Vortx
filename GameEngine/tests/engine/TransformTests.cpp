#include "TestUtils.h"
#include "Components/ComponentTransform.h"
#include "SimpleAPI/Math.h"

using namespace Vortx;

class TransformTest : public SceneTest {};

TEST_F(TransformTest, Defaults)
{
    Transform t = Tr(Make());
    EXPECT_VEC2_NEAR(t.Position(), glm::vec2(0, 0), 1e-6f);
    EXPECT_NEAR(t.Rotation(), 0.0f, 1e-6f);
    EXPECT_VEC2_NEAR(t.Scale(), glm::vec2(1, 1), 1e-6f);
    EXPECT_EQ(t.GetChildCount(), 0);
    EXPECT_FALSE(t.GetParent().has_value());
}

TEST_F(TransformTest, LocalSettersAreReflectedInWorldWithoutParent)
{
    Transform t = Tr(Make());
    t.SetLocalPosition({ 3, 4 });
    t.SetLocalRotation(0.5f);
    t.SetLocalScale({ 2, 3 });

    EXPECT_VEC2_NEAR(t.LocalPosition(), glm::vec2(3, 4), 1e-6f);
    EXPECT_VEC2_NEAR(t.Position(), glm::vec2(3, 4), 1e-5f);
    EXPECT_NEAR(t.Rotation(), 0.5f, 1e-6f);
    EXPECT_VEC2_NEAR(t.Scale(), glm::vec2(2, 3), 1e-6f);
}

TEST_F(TransformTest, WorldSettersWithoutParentMatchLocal)
{
    Transform t = Tr(Make());
    t.SetPosition({ 1, 2 });
    t.SetRotation(1.0f);
    t.SetScale({ 4, 5 });

    EXPECT_VEC2_NEAR(t.LocalPosition(), glm::vec2(1, 2), 1e-5f);
    EXPECT_NEAR(t.LocalRotation(), 1.0f, 1e-6f);
    EXPECT_VEC2_NEAR(t.LocalScale(), glm::vec2(4, 5), 1e-6f);
}

TEST_F(TransformTest, RightAndUpFollowRotation)
{
    Transform t = Tr(Make());
    EXPECT_VEC2_NEAR(t.Right(), glm::vec2(1, 0), 1e-6f);
    EXPECT_VEC2_NEAR(t.Up(), glm::vec2(0, 1), 1e-6f);

    t.SetRotation(Math::Pi / 2);
    EXPECT_VEC2_NEAR(t.Right(), glm::vec2(0, 1), 1e-5f);
    EXPECT_VEC2_NEAR(t.Up(), glm::vec2(-1, 0), 1e-5f);
}

TEST_F(TransformTest, TranslateIsWorldSpaceAndLocalTranslateIsLocal)
{
    Transform t = Tr(Make());
    t.Translate({ 1, 0 });
    t.LocalTranslate({ 0, 2 });
    EXPECT_VEC2_NEAR(t.Position(), glm::vec2(1, 2), 1e-5f);
}

TEST_F(TransformTest, RotateAddsToLocalRotation)
{
    Transform t = Tr(Make());
    t.Rotate(0.25f);
    t.Rotate(0.25f);
    EXPECT_NEAR(t.LocalRotation(), 0.5f, 1e-6f);
}

TEST_F(TransformTest, LookAtPointsRightAxisAtTarget)
{
    Transform t = Tr(Make());
    t.LookAt({ 0, 5 });
    EXPECT_NEAR(t.Rotation(), Math::Pi / 2, 1e-5f);
    EXPECT_VEC2_NEAR(t.Right(), glm::vec2(0, 1), 1e-5f);
}

TEST_F(TransformTest, LocalWorldPointConversionRoundTrips)
{
    Transform t = Tr(Make());
    t.SetPosition({ 1, 1 });
    t.SetRotation(Math::Pi / 2);
    t.SetScale({ 2, 2 });

    glm::vec2 world = t.LocalToWorldPoint({ 1, 0 });
    EXPECT_VEC2_NEAR(world, glm::vec2(1, 3), 1e-4f);
    EXPECT_VEC2_NEAR(t.WorldToLocalPoint(world), glm::vec2(1, 0), 1e-4f);
}

TEST_F(TransformTest, LocalToWorldDirectionIgnoresTranslation)
{
    Transform t = Tr(Make());
    t.SetPosition({ 10, 10 });
    t.SetScale({ 2, 2 });
    EXPECT_VEC2_NEAR(t.LocalToWorldDirection({ 1, 0 }), glm::vec2(2, 0), 1e-5f);
}

TEST_F(TransformTest, InverseWorldMatrixInvertsWorldMatrix)
{
    Transform t = Tr(Make());
    t.SetPosition({ 3, -2 });
    t.SetRotation(0.8f);
    t.SetScale({ 2, 2 });

    glm::mat4 m = t.GetInverseWorldMatrix() * t.GetWorldMatrix();
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            EXPECT_NEAR(m[c][r], c == r ? 1.0f : 0.0f, 1e-4f);
}

TEST_F(TransformTest, SetParentPreservesWorldTransform)
{
    Transform parent = Tr(Make("parent"));
    Transform child = Tr(Make("child"));
    parent.SetPosition({ 10, 0 });
    parent.SetScale({ 2, 2 });
    child.SetPosition({ 14, 2 });

    child.SetParent(parent);

    EXPECT_VEC2_NEAR(child.Position(), glm::vec2(14, 2), 1e-4f);
    EXPECT_VEC2_NEAR(child.LocalPosition(), glm::vec2(2, 1), 1e-4f);
    ASSERT_TRUE(child.GetParent().has_value());
    EXPECT_TRUE(*child.GetParent() == parent);
    EXPECT_EQ(parent.GetChildCount(), 1);
    EXPECT_TRUE(parent.GetChild(0) == child);
}

TEST_F(TransformTest, MovingParentMovesChild)
{
    Transform parent = Tr(Make());
    Transform child = Tr(Make());
    child.SetParent(parent);
    child.SetLocalPosition({ 1, 0 });

    parent.SetPosition({ 5, 5 });
    EXPECT_VEC2_NEAR(child.Position(), glm::vec2(6, 5), 1e-4f);
}

TEST_F(TransformTest, RotatingParentRotatesChildPosition)
{
    Transform parent = Tr(Make());
    Transform child = Tr(Make());
    child.SetParent(parent);
    child.SetLocalPosition({ 1, 0 });

    parent.SetRotation(Math::Pi / 2);
    EXPECT_VEC2_NEAR(child.Position(), glm::vec2(0, 1), 1e-4f);
    EXPECT_NEAR(child.Rotation(), Math::Pi / 2, 1e-5f);
}

TEST_F(TransformTest, UnparentingMovesBackToSceneRoot)
{
    Transform parent = Tr(Make());
    Transform child = Tr(Make());
    child.SetParent(parent);
    EXPECT_EQ(scene().GetChildren().size(), 1u);

    child.SetParent(std::nullopt);
    EXPECT_FALSE(child.GetParent().has_value());
    EXPECT_EQ(parent.GetChildCount(), 0);
    EXPECT_EQ(scene().GetChildren().size(), 2u);
}

TEST_F(TransformTest, SetParentAtIndexOrdersChildren)
{
    Transform parent = Tr(Make());
    Transform a = Tr(Make("a"));
    Transform b = Tr(Make("b"));
    Transform c = Tr(Make("c"));

    a.SetParent(parent);
    b.SetParent(parent);
    c.SetParent(parent, 0);

    EXPECT_EQ(parent.GetChild(0).GetGameObject().NameGet(), "c");
    EXPECT_EQ(parent.GetChild(1).GetGameObject().NameGet(), "a");
    EXPECT_EQ(parent.GetChild(2).GetGameObject().NameGet(), "b");
}

TEST_F(TransformTest, GetChildOutOfRangeReturnsDefault)
{
    Transform t = Tr(Make());
    EXPECT_TRUE(t.GetChild(0) == Transform());
    EXPECT_TRUE(t.GetChild(-1) == Transform());
}

TEST_F(TransformTest, UpdateSubtreeClearsDirtyFlags)
{
    Transform parent = Tr(Make());
    Transform child = Tr(Make());
    child.SetParent(parent);
    parent.SetPosition({ 1, 1 });

    parent.UpdateSubtree();
    EXPECT_FALSE(parent.Data().worldDirty);
    EXPECT_FALSE(child.Data().worldDirty);
}

TEST_F(TransformTest, EqualityComparesEntities)
{
    GameObject a = Make();
    GameObject b = Make();
    EXPECT_TRUE(Tr(a) == Tr(a));
    EXPECT_TRUE(Tr(a) != Tr(b));
}
