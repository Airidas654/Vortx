#pragma once

#include <glm/glm.hpp>

namespace Vortx
{
    inline float Cross(const glm::vec2& a, const glm::vec2& b)
    {
        return a.x * b.y - a.y * b.x;
    }

    inline glm::vec2 Cross(float s, const glm::vec2& v)
    {
        return { -s * v.y, s * v.x };
    }

    inline glm::vec2 Rotate(const glm::vec2& v, float angle)
    {
        float c = cosf(angle), s = sinf(angle);
        return { v.x * c - v.y * s, v.x * s + v.y * c };
    }
}