#pragma once

#include <functional>

#include "mesh.h"
#include "glm/common.hpp"

namespace fragment_shader
{
    struct FragmentUniforms
    {
        float nearPlane;
        float farPlane;

        // Linearized Depth Mapping
        float minDepth;
        float maxDepth;

        rasterizer::Texture texture;
    };

    using FragmentShaderPtr = std::function<rasterizer::color4ub(const rasterizer::Fragment&, const FragmentUniforms&)>;
    struct FragmentShader
    {
        FragmentUniforms uniforms;
        FragmentShaderPtr functor = nullptr;

        rasterizer::color4ub operator()(const rasterizer::Fragment& f) const
        {
            if (functor == nullptr)
                return rasterizer::ToColor4UB(f.color);

            return functor(f, uniforms);
        }
    };

    inline rasterizer::color4ub FragmentShaderFlat(const rasterizer::Fragment& fragment, const FragmentUniforms& uniforms)
    {
        rasterizer::color4ub result = rasterizer::ToColor4UB(fragment.color);
        return result;
    }

    inline rasterizer::color4ub FragmentShaderTexture(const rasterizer::Fragment& fragment, const FragmentUniforms& uniforms)
    {
        int x = fragment.uv.x * uniforms.texture.width;
        int y = fragment.uv.y * uniforms.texture.height;

        rasterizer::color4ub result = rasterizer::ToColor4UB(fragment.color);
        size_t sample_index = x + y * uniforms.texture.width;
        if (sample_index < uniforms.texture.pixels.size())
        {
            result = uniforms.texture.pixels[sample_index];
        }

        return result;
    }

    inline rasterizer::color4ub FragmentShaderDepthMap(const rasterizer::Fragment& fragment, const FragmentUniforms& uniforms)
    {
        glm::vec4 color = fragment.color;

        const float trueDepth = 2*uniforms.nearPlane*uniforms.farPlane/(uniforms.nearPlane + uniforms.farPlane - fragment.depth*(uniforms.farPlane - uniforms.nearPlane));
        float colorIntensity = (trueDepth - uniforms.minDepth)/(uniforms.maxDepth - uniforms.minDepth);
        colorIntensity = glm::clamp(colorIntensity, 0.0f, 1.0f);
        colorIntensity = 1 - colorIntensity;

        color.x = colorIntensity;
        color.y = colorIntensity;
        color.z = colorIntensity;

        return rasterizer::ToColor4UB(color);
    }
}
