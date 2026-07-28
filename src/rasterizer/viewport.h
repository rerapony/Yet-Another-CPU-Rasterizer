#pragma once

struct Viewport
{
    Viewport(const int width, const int height) : width_(width), height_(height) {}

    float GetAspectRatio() const
    {
        return height_ > 0 ? static_cast<float>(width_) / static_cast<float>(height_) : 1.0f;
    }

    int GetWidth() const { return width_; }
    int GetHeight() const { return height_; }

private:
    int width_;
    int height_;
};