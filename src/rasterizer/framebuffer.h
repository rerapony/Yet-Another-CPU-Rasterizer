#pragma once

#include <cfloat>
#include <vector>

#include "types.h"

class Framebuffer
{
public:
    Framebuffer(const size_t width, const size_t height) : width_(width), height_(height)
    {
        color_buffer_.resize(width * height);
        z_buffer_.resize(width * height);
    }

    void SetPixel(const int x, const int y, const rasterizer::color4ub& color)
    {
        color_buffer_[x + y * width_] = color;
    }

    void SetDepth(const int x, const int y, float depth)
    {
        z_buffer_[x + y * width_] = depth;
    }

    float GetDepth(const int x, const int y) const { return z_buffer_[x + y * width_]; }

    void Clear(const rasterizer::color4ub& color)
    {
        std::ranges::fill(color_buffer_, color);
        std::ranges::fill(z_buffer_, FLT_MAX);
    }

    std::vector<rasterizer::color4ub>& GetColorBuffer() { return color_buffer_; }
    size_t GetPitch() const { return width_ * sizeof(rasterizer::color4ub); }

private:
    std::vector<rasterizer::color4ub> color_buffer_;
    std::vector<float> z_buffer_;
    float minDepth_ = FLT_MAX;
    float maxDepth_ = FLT_MIN;
    int width_ = 0;
    int height_ = 0;
};
