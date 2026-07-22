#include "EditorController.h"

#include "Scene.h"
#include "ECS/Entity.h"
#include "SimpleAPI/GameObject.h"
#include "SimpleAPI/Transform.h"
#include "Components/ComponentTransform.h"
#include "Components/ComponentSpriteRenderer.h"
#include "Components/ComponentMetadata.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "imgui.h"

#include <algorithm>
#include <cmath>

using namespace Vortx;

namespace
{
    constexpr float kPi = 3.14159265358979323846f;

    float DistancePointSegment(const glm::vec2& p, const glm::vec2& a, const glm::vec2& b)
    {
        glm::vec2 ab = b - a;
        float lenSq = glm::dot(ab, ab);
        float t = lenSq > 0.0f ? std::clamp(glm::dot(p - a, ab) / lenSq, 0.0f, 1.0f) : 0.0f;
        glm::vec2 closest = a + ab * t;
        return glm::length(p - closest);
    }

    void DrawArrowHead(ImDrawList* dl, const ImVec2& from, const ImVec2& to, ImU32 color)
    {
        ImVec2 dir = { to.x - from.x, to.y - from.y };
        float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
        if (len < 0.0001f) return;

        dir.x /= len;
        dir.y /= len;
        ImVec2 perp = { -dir.y, dir.x };

        const float headLen = 10.0f;
        const float headWidth = 6.0f;

        ImVec2 base = { to.x - dir.x * headLen, to.y - dir.y * headLen };
        ImVec2 p1 = { base.x + perp.x * headWidth, base.y + perp.y * headWidth };
        ImVec2 p2 = { base.x - perp.x * headWidth, base.y - perp.y * headWidth };

        dl->AddTriangleFilled(to, p1, p2, color);
    }

    void DrawHandleBox(ImDrawList* dl, const ImVec2& center, float half, ImU32 color)
    {
        dl->AddRectFilled({ center.x - half, center.y - half }, { center.x + half, center.y + half }, color);
    }
}

void EditorController::Update(GLFWwindow* window, bool viewportHovered, const glm::vec2& localMousePos,
    const glm::vec2& viewportSize, float unscaledDeltaTime, const AssetID& selected)
{
    bool rightMouseDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    bool leftMouseDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    if (rightMouseDown && viewportHovered && !wasRightMouseDown)
        isPanning = true;
    else if (!rightMouseDown)
        isPanning = false;

    // 1 / 2 / 3 switch the active gizmo. Gated on hovering the Scene panel
    // (not WantCaptureKeyboard, which stays true almost constantly with
    // keyboard nav enabled).
    if (!isPanning && viewportHovered)
    {
        bool key1Down = glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;
        bool key2Down = glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS;
        bool key3Down = glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS;

        if (key1Down && !wasKey1Down) gizmoMode = GizmoMode::Move;
        if (key2Down && !wasKey2Down) gizmoMode = GizmoMode::Rotate;
        if (key3Down && !wasKey3Down) gizmoMode = GizmoMode::Scale;

        wasKey1Down = key1Down;
        wasKey2Down = key2Down;
        wasKey3Down = key3Down;
    }

    float pixelsToWorld = (viewportSize.y > 0.0f) ? (viewSize * 2.0f) / viewportSize.y : 0.0f;
    glm::vec2 mouseDeltaPixels = localMousePos - lastMousePos;
    glm::vec2 mouseDeltaWorld = { mouseDeltaPixels.x * pixelsToWorld, -mouseDeltaPixels.y * pixelsToWorld };

    
    float speed = panSpeed * viewSize * unscaledDeltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) position.y += speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) position.y -= speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) position.x -= speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) position.x += speed;

    if (wasRightMouseDown)
        position -= mouseDeltaWorld;

    if (leftMouseDown && !wasLeftMouseDown && viewportHovered && !isPanning)
    {
        GizmoElement hitGizmo = (selected != AssetID(0))
            ? HitTestGizmo(selected, localMousePos, viewportSize)
            : GizmoElement::None;

        if (hitGizmo != GizmoElement::None)
        {
            activeGizmo = hitGizmo;
            isDraggingObject = false;

            TransformData* t = GetTransform(selected);
            glm::vec2 objLocal = t ? WorldToLocal(t->position, viewportSize) : glm::vec2(0.0f);

            if (hitGizmo == GizmoElement::Rotate)
                lastMouseAngle = atan2f(-(localMousePos.y - objLocal.y), localMousePos.x - objLocal.x);
            else if (hitGizmo == GizmoElement::ScaleFree)
                lastScaleDistPixels = glm::length(localMousePos - objLocal);
        }
        else
        {
            activeGizmo = GizmoElement::None;

            AssetID hit = Pick(localMousePos, viewportSize);
            if (onSelect) onSelect(hit);

            if (hit != AssetID(0))
            {
                isDraggingObject = true;
                draggedObject = hit;
                pressLocalPos = localMousePos;
                dragConfirmed = false;
            }
            else
            {
                isDraggingObject = false;
            }
        }
    }
    else if (!leftMouseDown)
    {
        isDraggingObject = false;
        dragConfirmed = false;
        activeGizmo = GizmoElement::None;
    }

    if (activeGizmo != GizmoElement::None && wasLeftMouseDown)
    {
        if (TransformData* t = GetTransform(selected))
        {
            switch (activeGizmo)
            {
            case GizmoElement::MoveX:
                t->position.x += mouseDeltaWorld.x;
                break;
            case GizmoElement::MoveY:
                t->position.y += mouseDeltaWorld.y;
                break;
            case GizmoElement::MoveFree:
                t->position += mouseDeltaWorld;
                break;
            case GizmoElement::Rotate:
            {
                glm::vec2 objLocal = WorldToLocal(t->position, viewportSize);
                float angle = atan2f(-(localMousePos.y - objLocal.y), localMousePos.x - objLocal.x);

                float delta = angle - lastMouseAngle;
                if (delta > kPi) delta -= 2.0f * kPi;
                if (delta < -kPi) delta += 2.0f * kPi;

                t->rotation.x += delta;
                lastMouseAngle = angle;
                break;
            }
            case GizmoElement::ScaleX:
                t->scale.x = std::max(minScale, t->scale.x + mouseDeltaWorld.x);
                break;
            case GizmoElement::ScaleY:
                t->scale.y = std::max(minScale, t->scale.y + mouseDeltaWorld.y);
                break;
            case GizmoElement::ScaleFree:
            {
                glm::vec2 objLocal = WorldToLocal(t->position, viewportSize);
                float distPixels = glm::length(localMousePos - objLocal);
                float deltaWorld = (distPixels - lastScaleDistPixels) * pixelsToWorld;

                t->scale.x = std::max(minScale, t->scale.x + deltaWorld);
                t->scale.y = std::max(minScale, t->scale.y + deltaWorld);

                lastScaleDistPixels = distPixels;
                break;
            }
            default:
                break;
            }
        }
        else
        {
            activeGizmo = GizmoElement::None;
        }
    }
    //// Freeform drag on the object body (no gizmo handle involved). Only
    //// moves the object once the cursor has traveled past a small threshold
    //// from the press point, so a plain click just selects without nudging it.
    //else if (isDraggingObject && wasLeftMouseDown && Scene::Current())
    //{
    //    if (!dragConfirmed && glm::length(localMousePos - pressLocalPos) >= dragThresholdPixels)
    //        dragConfirmed = true;

    //    if (dragConfirmed)
    //    {
    //        auto& registry = Scene::Current()->gameObjectsRegister;
    //        auto it = registry.find(draggedObject);

    //        if (it != registry.end())
    //        {
    //            GameObject go = it->second;
    //            go.GetComponent<TransformData>().position += mouseDeltaWorld;
    //        }
    //        else
    //        {
    //            isDraggingObject = false;
    //        }
    //    }
    //}

    if (viewportHovered)
    {
        float scroll = ImGui::GetIO().MouseWheel;
        if (scroll != 0.0f)
        {
            viewSize -= scroll * (viewSize / zoomSpeed);
            viewSize = std::clamp(viewSize, minViewSize, maxViewSize);
        }
    }

    wasRightMouseDown = rightMouseDown;
    wasLeftMouseDown = leftMouseDown;
    lastMousePos = localMousePos;
}

void EditorController::DrawGizmo(ImDrawList* drawList, const glm::vec2& screenOrigin, const glm::vec2& viewportSize,
    const AssetID& selected) const
{
    if (!drawList || selected == AssetID(0)) return;

    TransformData* t = GetTransform(selected);
    if (!t) return;

    glm::vec2 objLocal = WorldToLocal(t->position, viewportSize);
    ImVec2 center = { screenOrigin.x + objLocal.x, screenOrigin.y + objLocal.y };

    glm::vec2 xHandle = objLocal + glm::vec2(axisLength, 0.0f);
    glm::vec2 yHandle = objLocal + glm::vec2(0.0f, -axisLength);
    ImVec2 xTip = { screenOrigin.x + xHandle.x, screenOrigin.y + xHandle.y };
    ImVec2 yTip = { screenOrigin.x + yHandle.x, screenOrigin.y + yHandle.y };

    const ImU32 highlight = IM_COL32(255, 235, 60, 255);
    ImU32 colX = (activeGizmo == GizmoElement::MoveX || activeGizmo == GizmoElement::ScaleX)
        ? highlight : IM_COL32(230, 60, 60, 255);
    ImU32 colY = (activeGizmo == GizmoElement::MoveY || activeGizmo == GizmoElement::ScaleY)
        ? highlight : IM_COL32(70, 200, 80, 255);
    ImU32 colFree = (activeGizmo == GizmoElement::MoveFree || activeGizmo == GizmoElement::ScaleFree)
        ? highlight : IM_COL32(235, 235, 235, 255);
    ImU32 colRotate = (activeGizmo == GizmoElement::Rotate) ? highlight : IM_COL32(70, 140, 240, 255);

    if (gizmoMode == GizmoMode::Rotate)
    {
        drawList->AddCircle(center, ringRadius, colRotate, 48, 2.0f);
        return;
    }

    drawList->AddLine(center, xTip, colX, 2.5f);
    drawList->AddLine(center, yTip, colY, 2.5f);

    if (gizmoMode == GizmoMode::Move)
    {
        DrawArrowHead(drawList, center, xTip, colX);
        DrawArrowHead(drawList, center, yTip, colY);
    }
    else
    {
        DrawHandleBox(drawList, xTip, scaleHandleHalf, colX);
        DrawHandleBox(drawList, yTip, scaleHandleHalf, colY);
    }

    DrawHandleBox(drawList, center, freeBoxHalf, colFree);
}

glm::mat4 EditorController::GetViewProjectionMatrix(float aspect) const
{
    float xSize = aspect * viewSize;

    glm::mat4 proj = glm::ortho(-xSize, xSize, -viewSize, viewSize, -1.0f, 1.0f);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(-position, 0.0f));

    return proj * view;
}

AssetID EditorController::Pick(const glm::vec2& localMousePos, const glm::vec2& viewportSize) const
{
    if (viewportSize.x <= 0.0f || viewportSize.y <= 0.0f) return AssetID(0);
    if (!Scene::Current()) return AssetID(0);

    float aspect = viewportSize.x / viewportSize.y;
    float xSize = aspect * viewSize;

    glm::vec2 ndc;
    ndc.x = (localMousePos.x / viewportSize.x) * 2.0f - 1.0f;
    ndc.y = 1.0f - (localMousePos.y / viewportSize.y) * 2.0f;

    glm::vec2 worldPos = position + glm::vec2(ndc.x * xSize, ndc.y * viewSize);

    AssetID hit(0);

    Scene::Current()->GetRegistry().View<SpriteRendererData, TransformData>().each(
        [&](entt::entity e, SpriteRendererData& sprite, TransformData& transform)
        {
            glm::vec2 local = worldPos - transform.position;

            float c = cosf(-transform.rotation.x);
            float s = sinf(-transform.rotation.x);
            glm::vec2 rotated = { local.x * c - local.y * s, local.x * s + local.y * c };

            glm::vec2 half = transform.scale * 0.5f;

            if (rotated.x >= -half.x && rotated.x <= half.x &&
                rotated.y >= -half.y && rotated.y <= half.y)
            {
                Entity entity(&Scene::Current()->GetRegistry(), e);
                hit = entity.GetComponent<MetadataData>().id;
            }
        });

    return hit;
}

EditorController::GizmoElement EditorController::HitTestGizmo(const AssetID& selected,
    const glm::vec2& localMousePos, const glm::vec2& viewportSize) const
{
    TransformData* t = GetTransform(selected);
    if (!t) return GizmoElement::None;

    glm::vec2 objLocal = WorldToLocal(t->position, viewportSize);

    if (gizmoMode == GizmoMode::Rotate)
    {
        float distToCenter = glm::length(localMousePos - objLocal);
        return (std::abs(distToCenter - ringRadius) <= pickTolerance) ? GizmoElement::Rotate : GizmoElement::None;
    }

    if (std::abs(localMousePos.x - objLocal.x) <= freeBoxHalf &&
        std::abs(localMousePos.y - objLocal.y) <= freeBoxHalf)
        return (gizmoMode == GizmoMode::Move) ? GizmoElement::MoveFree : GizmoElement::ScaleFree;

    glm::vec2 xHandle = objLocal + glm::vec2(axisLength, 0.0f);
    glm::vec2 yHandle = objLocal + glm::vec2(0.0f, -axisLength);

    if (DistancePointSegment(localMousePos, objLocal, xHandle) <= pickTolerance)
        return (gizmoMode == GizmoMode::Move) ? GizmoElement::MoveX : GizmoElement::ScaleX;

    if (DistancePointSegment(localMousePos, objLocal, yHandle) <= pickTolerance)
        return (gizmoMode == GizmoMode::Move) ? GizmoElement::MoveY : GizmoElement::ScaleY;

    return GizmoElement::None;
}

TransformData* EditorController::GetTransform(const AssetID& id) const
{
    if (!Scene::Current() || id == AssetID(0)) return nullptr;

    auto& registry = Scene::Current()->gameObjectsRegister;
    auto it = registry.find(id);
    if (it == registry.end()) return nullptr;

    GameObject go = it->second;
    return &go.GetComponent<TransformData>();
}

glm::vec2 EditorController::WorldToLocal(const glm::vec2& worldPos, const glm::vec2& viewportSize) const
{
    float aspect = (viewportSize.y > 0.0f) ? viewportSize.x / viewportSize.y : 1.0f;
    float xSize = aspect * viewSize;

    glm::vec2 ndc;
    ndc.x = (xSize > 0.0f) ? (worldPos.x - position.x) / xSize : 0.0f;
    ndc.y = (viewSize > 0.0f) ? (worldPos.y - position.y) / viewSize : 0.0f;

    glm::vec2 local;
    local.x = (ndc.x + 1.0f) * 0.5f * viewportSize.x;
    local.y = (1.0f - ndc.y) * 0.5f * viewportSize.y;
    return local;
}