#pragma once

#include <memory>

#include "framebuffer.h"
#include "mesh.h"
#include "render_state.h"
#include "viewport.h"


class Rasterizer
{
public:
    Rasterizer(Framebuffer* framebuffer);
    void SetRenderingSettings(const RenderState& settings);
    void SetViewport(const Viewport& viewport);

    void Clear(const rasterizer::color4ub& color);
    void Draw(const rasterizer::Mesh& mesh) const;

private:
    void DrawTriangle(rasterizer::Vertex v0, rasterizer::Vertex v1, rasterizer::Vertex v2) const;

    rasterizer::point NDCToViewport(const rasterizer::point& p) const;

    std::shared_ptr<Framebuffer> framebuffer_ = nullptr;
    RenderState settings_;
    Viewport viewport_;
};
