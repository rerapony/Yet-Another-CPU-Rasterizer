#pragma once

#include <cmath>

struct Viewport
{
    float xmin, xmax, ymin, ymax;

    float GetAspectRatio() const {
        float width = std::abs(xmax - xmin);
        float height = std::abs(ymax - ymin);
        return (height > 0.0f) ? (width / height) : 1.0f;
    }
};