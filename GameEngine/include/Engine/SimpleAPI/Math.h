#pragma once

#include <glm/glm.hpp>

#define _USE_MATH_DEFINES

#include <math.h>

namespace Vortx::Math
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

    inline float Distance(const glm::vec2& a, const glm::vec2& b)
    {
        return glm::distance(a, b);
    }

    static constexpr float Pi = M_PI;

    static constexpr float RadToDeg = 180.0f / Pi;
    static constexpr float DegToRad = Pi / 180.0f;
}