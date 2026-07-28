#pragma once

#include <functional>

#include "camera.h"
#include "mesh.h"
#include "glm/matrix.hpp"

namespace vertex_shader
{
    struct VertexUniforms
    {
        glm::mat4 mvp;
    };

    using VertexShaderPtr = std::function<rasterizer::point(const rasterizer::point&, const VertexUniforms&)>;
    struct VertexShader
    {
        VertexUniforms uniforms;
        VertexShaderPtr functor = nullptr;

        rasterizer::point operator()(const rasterizer::point& p) const
        {
            if (functor == nullptr)
                return p;

            return functor(p, uniforms);
        }
    };

    inline rasterizer::point VertexShaderMVP(const rasterizer::point& p, const VertexUniforms& uniforms)
    {
        return uniforms.mvp * p;
    }

    glm::mat4 NormalizedModelMatrix(const rasterizer::BoundingBox& aabb, float targetSize, float rotY = 0.0f);
    glm::mat4 WorldToCameraMatrix(const Camera& camera, const glm::vec3& targetPosition, const glm::vec3& upVector = glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 PerspectiveMatrix(const Camera& camera, float aspectRatio, float near, float far);;
}
