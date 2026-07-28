#pragma once

#include <functional>

#include "mesh.h"
#include "glm/common.hpp"

namespace fragment_shader
{
    struct FragmentUniforms
    {
        rasterizer::Texture texture;
    };

    struct Fragment
    {
        glm::vec4 color = glm::vec4(1.f);
        float depth;
        float u, v;
    };

    using FragmentShaderPtr = std::function<rasterizer::color4ub(const Fragment&, const FragmentUniforms&)>;
    struct FragmentShader
    {
        FragmentUniforms uniforms;
        FragmentShaderPtr functor = nullptr;

        rasterizer::color4ub operator()(const Fragment& f) const
        {
            if (functor == nullptr)
                return rasterizer::ToColor4UB(f.color);

            return functor(f, uniforms);
        }
    };

    inline rasterizer::color4ub FragmentShaderFlat(const Fragment& fragment, const FragmentUniforms& uniforms)
    {
        rasterizer::color4ub result = rasterizer::ToColor4UB(fragment.color);
        return result;
    }

    inline rasterizer::color4ub FragmentShaderTexture(const Fragment& fragment, const FragmentUniforms& uniforms)
    {
        const int u = fragment.u * uniforms.texture.width;
        const int v = fragment.v * uniforms.texture.height;

        rasterizer::color4ub result = rasterizer::ToColor4UB(fragment.color);
        size_t sample_index = u + v * uniforms.texture.width;
        sample_index = std::min(sample_index, uniforms.texture.pixels.size() - 1);
        result = uniforms.texture.pixels[sample_index];

        return result;
    }
}
