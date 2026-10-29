#include <gtest/gtest.h>
#include "EditorController.h"
#include "GizmosManager.h"
#include "SimpleAPI/Gizmos.h"

#include <glm/glm.hpp>

// Update()/DrawGizmo() need a GLFW window and an ImGui draw list, so only the
// state/maths surface is unit-tested here. Interactive behaviour (picking,
// dragging gizmos) is better covered by an integration/manual pass.

TEST(EditorController, Defaults)
{
    EditorController c;
    EXPECT_FLOAT_EQ(c.GetPosition().x, 0.0f);
    EXPECT_FLOAT_EQ(c.GetPosition().y, 0.0f);
    EXPECT_FLOAT_EQ(c.GetViewSize(), 5.0f);
    EXPECT_FALSE(c.IsPanning());
    EXPECT_FALSE(c.IsDraggingObject());
    EXPECT_EQ(c.ActiveGizmo(), EditorController::GizmoElement::None);
    EXPECT_EQ(c.GetGizmoMode(), EditorController::GizmoMode::Move);
    EXPECT_EQ(c.GetGizmoSpaceMode(), EditorController::SpaceMode::Local);
    EXPECT_FALSE(static_cast<bool>(c.onSelect));
}

TEST(EditorController, ModeSetters)
{
    EditorController c;
    c.SetGizmoMode(EditorController::GizmoMode::Scale);
    c.SetGizmoSpaceMode(EditorController::SpaceMode::World);
    EXPECT_EQ(c.GetGizmoMode(), EditorController::GizmoMode::Scale);
    EXPECT_EQ(c.GetGizmoSpaceMode(), EditorController::SpaceMode::World);
}

TEST(EditorController, ViewProjectionMapsViewCornersToClipSpace)
{
    EditorController c;   // viewSize 5, camera at origin
    glm::mat4 vp = c.GetViewProjectionMatrix(2.0f);   // x half-extent = 10

    glm::vec4 topRight = vp * glm::vec4(10, 5, 0, 1);
    glm::vec4 bottomLeft = vp * glm::vec4(-10, -5, 0, 1);
    glm::vec4 center = vp * glm::vec4(0, 0, 0, 1);

    EXPECT_NEAR(topRight.x, 1.0f, 1e-5f);
    EXPECT_NEAR(topRight.y, 1.0f, 1e-5f);
    EXPECT_NEAR(bottomLeft.x, -1.0f, 1e-5f);
    EXPECT_NEAR(bottomLeft.y, -1.0f, 1e-5f);
    EXPECT_NEAR(center.x, 0.0f, 1e-5f);
    EXPECT_NEAR(center.y, 0.0f, 1e-5f);
}

TEST(EditorController, ViewProjectionAspectScalesHorizontalOnly)
{
    EditorController c;
    glm::mat4 a = c.GetViewProjectionMatrix(1.0f);
    glm::mat4 b = c.GetViewProjectionMatrix(2.0f);
    EXPECT_NEAR(a[0][0], 2.0f * b[0][0], 1e-6f);
    EXPECT_NEAR(a[1][1], b[1][1], 1e-6f);
}

TEST(GizmosManager, ConstructorEnablesGizmoRecording)
{
    Vortx::Gizmos::enabled = false;
    GizmosManager manager;
    EXPECT_TRUE(Vortx::Gizmos::enabled);
    Vortx::Gizmos::enabled = false;
}
