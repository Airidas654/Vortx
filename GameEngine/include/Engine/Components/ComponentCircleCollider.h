#pragma once

#include "Physics/PhysicsTypes.h"

#include <glm/glm.hpp>

namespace Vortx
{
    struct CircleColliderData
    {
        glm::vec2 offset{ 0.0f };
        float radius = 0.5f;

        float density = 1.0f;
        bool isTrigger = false;

        PhysicsMaterial material;
        CollisionFilter filter;
    };
}