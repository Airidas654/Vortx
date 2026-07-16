#include "EditorController.h"

#include "Scene.h"
#include "ECS/Entity.h"
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

void EditorController::Update(GLFWwindow* window, bool viewportHovered, float unscaledDeltaTime)
{
    bool rightMouseDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

    // Only start panning if the button was pressed while hovering the
    // viewport, so a right-click elsewhere in the editor doesn't suddenly
    // start moving the camera once the cursor drifts over the Scene panel.
    if (rightMouseDown && viewportHovered)
        isPanning = true;
    else if (!rightMouseDown)
        isPanning = false;

   // if (isPanning)
    //{
        float speed = panSpeed * viewSize * unscaledDeltaTime;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) position.y += speed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) position.y -= speed;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) position.x -= speed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) position.x += speed;
   // }

    if (viewportHovered)
    {
        float scroll = ImGui::GetIO().MouseWheel;
        if (scroll != 0.0f)
        {
            viewSize -= scroll * (viewSize / zoomSpeed);
            viewSize = std::clamp(viewSize, minViewSize, maxViewSize);
        }
    }
}

glm::mat4 EditorController::GetViewProjectionMatrix(float aspect) const
{
    float xSize = aspect * viewSize;

    glm::mat4 proj = glm::ortho(-xSize, xSize, -viewSize, viewSize, -1.0f, 1.0f);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(-position, 0.0f));

    return proj * view;
}

void EditorController::HandleClick(const glm::vec2& localMousePos, const glm::vec2& viewportSize)
{
    if (viewportSize.x <= 0.0f || viewportSize.y <= 0.0f) return;
    if (!Scene::Current()) return;

    float aspect = viewportSize.x / viewportSize.y;
    float xSize = aspect * viewSize;

    // Pixel -> NDC (y flipped, since (0,0) is the top-left of the image).
    glm::vec2 ndc;
    ndc.x = (localMousePos.x / viewportSize.x) * 2.0f - 1.0f;
    ndc.y = 1.0f - (localMousePos.y / viewportSize.y) * 2.0f;

    glm::vec2 worldPos = position + glm::vec2(ndc.x * xSize, ndc.y * viewSize);

    AssetID hit(0);

    // Walk every sprite/quad and keep the last (topmost-drawn) one whose
    // bounds contain the click. This mirrors the draw order used by
    // DrawSceneSprites, so the visually topmost object wins ties.
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

    if (onSelect) onSelect(hit);
}