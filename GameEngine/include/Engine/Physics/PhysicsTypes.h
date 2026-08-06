#pragma once

#include "EngineAPI.h"

#include <glm/glm.hpp>
#include <cstdint>

namespace Vortx
{
    enum class BodyType : uint8_t
    {
        Static,
        Kinematic,
        Dynamic
    };

    struct PhysicsMaterial
    {
        float friction = 0.3f;
        float restitution = 0.0f;
        
        float restitutionThreshold = 0.5f;
    };

    struct CollisionFilter
    {
        uint32_t categoryBits = 0x0001;
        uint32_t maskBits = 0xFFFFFFFF;
        int16_t groupIndex = 0;
    };
}