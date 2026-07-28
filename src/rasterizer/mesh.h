#pragma once
#include <cfloat>
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

    // AABB (Axis-Aligned Bounding Box)
    struct BoundingBox
    {
        glm::vec3 minBound, maxBound;

        void Reset()
        {
            minBound = glm::vec3(FLT_MAX);
            maxBound = glm::vec3(-FLT_MAX);
        }
    };

    struct Mesh
    {
        std::vector<float> x, y, z;
        std::vector<float> u, v;
        std::vector<glm::vec4> colors;

        std::vector<size_t> v_indices;
        std::vector<size_t> vt_indices;

        size_t primitives_num = 0;

        // for scaling
        BoundingBox AABB;

        void Reset()
        {
            x.clear();
            y.clear();
            z.clear();
            u.clear();
            v.clear();
            colors.clear();

            v_indices.clear();
            vt_indices.clear();

            AABB.Reset();
        }
    };
}