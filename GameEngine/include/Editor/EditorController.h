#pragma once

#include "AssetID.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <functional>

struct GLFWwindow;

// Simple fly-style controller for the Scene view.
//
// Controls:
//   - Hold Right Mouse Button while hovering the Scene panel to enable camera
//     movement, then W/A/S/D pans the camera up/left/down/right.
//   - Mouse wheel while hovering the Scene panel zooms in/out.
//   - Left click (when not panning) selects the topmost sprite/quad under the
//     cursor. Clicking empty space clears the selection.
//
// This class owns no rendering state - it only tracks camera position/zoom
// and turns raw input into a view-projection matrix and a pick result, so it
// can be dropped into EditorUI's existing Scene panel with a few call sites.
class EditorController
{
public:
    EditorController() = default;

    // Call once per frame (typically from DrawSceneViewPanel) whether or not
    // the panel is hovered - hover state gates which inputs are read.
    void Update(GLFWwindow* window, bool viewportHovered, float unscaledDeltaTime);

    // localMousePos: mouse position relative to the top-left of the Scene
    // image, in pixels (0,0 = top-left). viewportSize: size of the Scene
    // image, in pixels. Call this when a left-click is detected on the image.
    void HandleClick(const glm::vec2& localMousePos, const glm::vec2& viewportSize);

    // Builds the combined view-projection matrix for the given aspect ratio
    // (viewport width / height), taking the current pan/zoom into account.
    glm::mat4 GetViewProjectionMatrix(float aspect) const;

    const glm::vec2& GetPosition() const { return position; }
    float GetViewSize() const { return viewSize; }
    bool IsPanning() const { return isPanning; }

    // Fired by HandleClick with the picked GameObject's AssetID, or
    // AssetID(0) if nothing was under the cursor.
    std::function<void(const Vortx::AssetID&)> onSelect;

private:
    glm::vec2 position{ 0.0f, 0.0f };
    float viewSize = 5.0f;

    float panSpeed = 4.0f;      // world units/sec at viewSize == 1
    float zoomSpeed = 10.0f;    // higher = slower zoom per scroll tick
    float minViewSize = 0.01f;
    float maxViewSize = 50.0f;

    bool isPanning = false;
};