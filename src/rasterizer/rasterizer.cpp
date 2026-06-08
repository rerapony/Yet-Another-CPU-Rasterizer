#include <algorithm>
#include <cassert>

#include "rasterizer.h"
#include "mesh.h"

#include <cmath>

#include "framebuffer.h"
#include "glm/detail/type_mat2x2.hpp"

using namespace rasterizer;

namespace
{
    float Det2D(const glm::vec4 &v0, const glm::vec4 &v1)
    {
        return glm::determinant(glm::mat2(v0.x, v0.y, v1.x, v1.y));
    }

    float EdgeFunction(const point &a, const point &b, const point &c)
    {
        return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
    }

    bool IsTopLeftEdge(const point &a, const point &b)
    {
        const float dx = b.x - a.x;
        const float dy = b.y - a.y;

        return dy > 0 || (dy == 0 && dx < 0);
    }
}

using namespace rasterizer;

Rasterizer::Rasterizer(Framebuffer* framebuffer) : framebuffer_(framebuffer)
{
}

void Rasterizer::SetRenderingSettings(const RenderState& settings)
{
    settings_ = settings;
}

void Rasterizer::SetViewport(const Viewport& viewport)
{
    viewport_ = viewport;
}

void Rasterizer::Clear(const color4ub& color)
{
    framebuffer_->Clear(color);
}

void Rasterizer::Draw(const Mesh& mesh) const
{
    for (Triangle triangle : mesh.triangles)
    {
        auto [i0, i1, i2] = triangle;

        Vertex v0(mesh.positions[i0.v], mesh.fragments[i0.vt]);
        Vertex v1(mesh.positions[i1.v], mesh.fragments[i1.vt]);
        Vertex v2(mesh.positions[i2.v], mesh.fragments[i2.vt]);

        DrawTriangle(v0, v1, v2);
    }
}

void Rasterizer::DrawTriangle(Vertex v0, Vertex v1, Vertex v2) const
{
    point p0 = settings_.vertexShader(AsPoint(v0.position.x, v0.position.y, v0.position.z));
    point p1 = settings_.vertexShader(AsPoint(v1.position.x, v1.position.y, v1.position.z));
    point p2 = settings_.vertexShader(AsPoint(v2.position.x, v2.position.y, v2.position.z));

    // simplified clipping
    // TODO: Near Clipping Plane
    if (p0.w <= 0.f || p1.w <= 0.f || p2.w <= 0.f)
        return;

    p0 = PerspectiveDivide(p0);
    p1 = PerspectiveDivide(p1);
    p2 = PerspectiveDivide(p2);

    p0 = NDCToViewport(p0);
    p1 = NDCToViewport(p1);
    p2 = NDCToViewport(p2);

    // perspective correct interpolation
    Fragment f0 = v0.fragment;
    Fragment f1 = v1.fragment;
    Fragment f2 = v2.fragment;
    float inv_w0 = 1.0f / p0.w;
    float inv_w1 = 1.0f / p1.w;
    float inv_w2 = 1.0f / p2.w;
    f0.uv *= inv_w0; f1.uv *= inv_w1; f2.uv *= inv_w2;
    f0.color *= inv_w0; f1.color *= inv_w1; f2.color *= inv_w2;

    // check the triangle orientation
    float e = EdgeFunction(p0, p1, p2); // area of the triangle multiplied by 2
    const bool isCCW = e > 0.f;
    if (!isCCW)
    {
        std::swap(v1, v2);
        std::swap(p1, p2);
        e = -e;
    }

    // culling triangles
    bool isFrontFace = settings_.windingOrder == CCW;
    switch (settings_.cullMode)
    {
    case Front:
        if (isFrontFace)
            return;
        break;
    case Back:
        if (!isFrontFace)
            return;
        break;
    default: ;
    }

    int xmin = std::max<float>(viewport_.xmin, 0);
    int xmax = std::min<float>(viewport_.xmax, framebuffer_->GetWidth()) - 1;
    int ymin = std::max<float>(viewport_.ymin, 0);
    int ymax = std::min<float>(viewport_.ymax, framebuffer_->GetHeight()) - 1;

    xmin = std::max<float>(xmin, std::min({std::floor(p0.x), std::floor(p1.x), std::floor(p2.x)}));
    xmax = std::min<float>(xmax, std::max({std::floor(p0.x), std::floor(p1.x), std::floor(p2.x)}));
    ymin = std::max<float>(ymin, std::min({std::floor(p0.y), std::floor(p1.y), std::floor(p2.y)}));
    ymax = std::min<float>(ymax, std::max({std::floor(p0.y), std::floor(p1.y), std::floor(p2.y)}));

    for (int y = ymin; y <= ymax; ++y)
    {
        for (int x = xmin; x <= xmax; ++x)
        {
            // pixel center
            point p = AsPoint({x + 0.5f, y + 0.5f, 0.f});

            float e0 = EdgeFunction(p1, p2, p);
            float e1 = EdgeFunction(p2, p0, p);
            float e2 = EdgeFunction(p0, p1, p);

            if (e0 >= 0.f && e1 >= 0.f && e2 >= 0.f)
            {
                e0 /= e;
                e1 /= e;
                e2 /= e;

                float Zndc = e0 * p0.z + e1 * p1.z + e2 * p2.z;
                if (Zndc < framebuffer_->GetDepth(x, y))
                {
                    framebuffer_->SetDepth(x, y, Zndc);
                    Fragment f;
                    f.depth = Zndc;

                    // perspective correct
                    float inv_w = e0 * inv_w0 + e1 * inv_w1 + e2 * inv_w2;
                    f.uv = e0 * f0.uv + e1 * f1.uv + e2 * f2.uv;
                    f.color = e0 * f0.color + e1 * f1.color + e2 * f2.color;
                    f.uv /= inv_w;
                    f.color /= inv_w;

                    framebuffer_->SetPixel(x, y, settings_.fragmentShader(f));
                }
            }
        }
    }
}

point Rasterizer::NDCToViewport(const point& p) const
{
    point result = p;
    result.x = viewport_.xmin + (viewport_.xmax - viewport_.xmin) * (0.5f + 0.5f * p.x);
    result.y = viewport_.ymin + (viewport_.ymax - viewport_.ymin) * (0.5f - 0.5f * p.y);

    return result;
}

