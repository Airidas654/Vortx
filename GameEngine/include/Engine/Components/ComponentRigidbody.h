#pragma once

#include "Physics/PhysicsTypes.h"

#include <glm/glm.hpp>

namespace Vortx
{
    struct RigidbodyData
    {
        BodyType bodyType = BodyType::Dynamic;

        float mass = 1.0f;
        bool useAutoMass = false;
        float inverseMass = 1.0f;
        float inertia = 1.0f;
        float inverseInertia = 1.0f;
        glm::vec2 centerOfMass{ 0.0f };

        glm::vec2 velocity{ 0.0f };
        float angularVelocity{ 0.0f };
        float gravityScale = 1.0f;
        float linearDamping = 0.0f;
        float angularDamping = 0.05f;
        bool fixedRotation = false;

        bool allowSleep = true;
        bool isAwake = true;
        float sleepTime = 0.0f;

        glm::vec2 forceAccum{ 0.0f };
        float torqueAccum{ 0.0f };

        bool massDirty = true;
    };
}