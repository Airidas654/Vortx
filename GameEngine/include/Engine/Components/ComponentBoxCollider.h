#pragma once

#include "Physics/PhysicsTypes.h"

#include <glm/glm.hpp>

namespace Vortx
{
    struct BoxColliderData
    {
        glm::vec2 offset{ 0.0f };
        glm::vec2 size{ 1.0f, 1.0f };

        float density = 1.0f;
        bool isTrigger = false;

        PhysicsMaterial material;
        CollisionFilter filter;
    };
}