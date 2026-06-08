#pragma once
#include <vector>

#include "types.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

namespace rasterizer
{
    struct Texture
    {
        int width;
        int height;
        std::vector<color4ub> pixels;
    };

    struct Fragment
    {
        glm::vec4 color = glm::vec4(1.f);
        float depth;
        glm::vec2 uv;
    };

    struct Position
    {
        float x, y, z;
    };

    struct Vertex
    {
        Position position{};
        Fragment fragment;
    };

    // indexed rendering
    struct VertexIndex
    {
        size_t v, vt, vn;
    };

    // indexed rendering
    struct Triangle
    {
        VertexIndex i0, i1, i2;
    };

    // AABB (Axis-Aligned Bounding Box)
    struct BoundingBox
    {
        glm::vec3 minBound, maxBound;
    };

    struct Mesh
    {
        std::vector<Position> positions;
        std::vector<Fragment> fragments;
        std::vector<Triangle> triangles;

        // for scaling
        BoundingBox AABB;
    };
}