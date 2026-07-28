#pragma once

#include "AssetID.h"
#include "SimpleAPI/GameObject.h"
#include "SimpleAPI/Transform.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <optional>

namespace Vortx
{

    struct TransformData
    {
        glm::vec2 localPosition;
        float localRotation;
        glm::vec2 localScale;

        glm::mat4 localMatrix = glm::mat4(1.0f);
        glm::mat4 worldMatrix = glm::mat4(1.0f);

        glm::vec2 worldPositionCache = glm::vec2(0);
        float worldRotationCache = 0.0f;
        glm::vec2 worldScaleCache = glm::vec2(1);

        bool worldDirty = true;

        std::optional<Transform> parent;
        std::vector<Transform> children;

        TransformData()
        {
            localPosition = glm::vec2(0);
            localRotation = 0;
            localScale = glm::vec2(1, 1);
            parent = std::nullopt;
        }
    };
}