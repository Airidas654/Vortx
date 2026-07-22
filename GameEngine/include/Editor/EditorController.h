#pragma once

#include "AssetID.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <functional>

struct GLFWwindow;
struct ImDrawList;

namespace Vortx { struct TransformData; }

class EditorController
{
public:
    enum class GizmoMode
    {
        Move,
        Rotate,
        Scale
    };

    enum class GizmoElement
    {
        None,
        MoveX,
        MoveY,
        MoveFree,
        Rotate,
        ScaleX,
        ScaleY,
        ScaleFree
    };

    EditorController() = default;

    void Update(GLFWwindow* window, bool viewportHovered, const glm::vec2& localMousePos,
        const glm::vec2& viewportSize, float unscaledDeltaTime, const Vortx::AssetID& selected);

    void DrawGizmo(ImDrawList* drawList, const glm::vec2& screenOrigin, const glm::vec2& viewportSize,
        const Vortx::AssetID& selected) const;

    glm::mat4 GetViewProjectionMatrix(float aspect) const;

    const glm::vec2& GetPosition() const { return position; }
    float GetViewSize() const { return viewSize; }
    bool IsPanning() const { return isPanning; }
    bool IsDraggingObject() const { return isDraggingObject; }
    GizmoElement ActiveGizmo() const { return activeGizmo; }

    GizmoMode GetGizmoMode() const { return gizmoMode; }
    void SetGizmoMode(GizmoMode mode) { gizmoMode = mode; }

    std::function<void(const Vortx::AssetID&)> onSelect;

private:
    glm::vec2 position{ 0.0f, 0.0f };
    float viewSize = 5.0f;

    float panSpeed = 4.0f;
    float zoomSpeed = 10.0f;
    float minViewSize = 0.5f;
    float maxViewSize = 50.0f;
    float minScale = 0.05f;

    float axisLength = 70.0f;
    float ringRadius = 55.0f;
    float pickTolerance = 8.0f;
    float freeBoxHalf = 6.0f;
    float scaleHandleHalf = 5.0f;

    GizmoMode gizmoMode = GizmoMode::Move;

    bool isPanning = false;

    bool isDraggingObject = false;
    Vortx::AssetID draggedObject{ 0 };

    GizmoElement activeGizmo = GizmoElement::None;
    float lastMouseAngle = 0.0f;
    float lastScaleDistPixels = 0.0f;

    bool wasRightMouseDown = false;
    bool wasLeftMouseDown = false;
    glm::vec2 lastMousePos{ 0.0f, 0.0f };

    bool wasKey1Down = false;
    bool wasKey2Down = false;
    bool wasKey3Down = false;

    // Click-vs-drag disambiguation: movement only counts as a drag once it
    // exceeds this many pixels from the initial press point, so a plain
    // click just selects.
    glm::vec2 pressLocalPos{ 0.0f, 0.0f };
    bool dragConfirmed = false;
    float dragThresholdPixels = 4.0f;

    Vortx::AssetID Pick(const glm::vec2& localMousePos, const glm::vec2& viewportSize) const;
    GizmoElement HitTestGizmo(const Vortx::AssetID& selected, const glm::vec2& localMousePos,
        const glm::vec2& viewportSize) const;

    Vortx::TransformData* GetTransform(const Vortx::AssetID& id) const;
    glm::vec2 WorldToLocal(const glm::vec2& worldPos, const glm::vec2& viewportSize) const;
};