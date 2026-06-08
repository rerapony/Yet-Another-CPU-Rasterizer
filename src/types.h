#pragma once

#include <algorithm>

#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

namespace rasterizer
{
    typedef glm::vec4 point;

    // homogeneous coordinates
    inline point AsPoint(const glm::vec3& v)
    {
        return {v.x, v.y, v.z, 1.f};
    }

    inline point AsPoint(float x, float y, float z)
    {
        return {x, y, z, 1.f};
    }

    inline glm::vec4 AsVector(const glm::vec3& v)
    {
        return {v.x, v.y, v.z, 0.f};
    }

    inline glm::vec4 PerspectiveDivide(const glm::vec4& v)
    {
        glm::vec4 result = v;

        result.x /= v.w;
        result.y /= v.w;
        result.z /= v.w;

        return result;
    }

    // aggregate
    struct color4ub
    {
        uint8_t r, g, b, a;
    };

    inline color4ub ToColor4UB(const glm::vec4& c)
    {
        color4ub result{};
        result.r = std::max(0.f, std::min(255.f, c.x * 255.f));
        result.g = std::max(0.f, std::min(255.f, c.y * 255.f));
        result.b = std::max(0.f, std::min(255.f, c.z * 255.f));
        result.a = std::max(0.f, std::min(255.f, c.w * 255.f));

        return result;
    }
}
