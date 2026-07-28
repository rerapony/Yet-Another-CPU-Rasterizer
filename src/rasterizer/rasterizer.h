#pragma once

#include <memory>

#include "framebuffer.h"
#include "mesh.h"
#include "render_state.h"
#include "viewport.h"

namespace rasterizer
{
    struct VertexBuffer
    {
        std::vector<float> x, y, z, w, inv_w;
        std::vector<float> u, v;

        void Clear();
        void Resize(size_t size);
    };

    struct RasterTriangle
    {
        size_t  i0, i1, i2;
        BoundingBox AABB;
        float e;
    };

    constexpr size_t TILE_SIZE = 32;

    struct Tile
    {
        int startX, startY;
        int endX, endY;
        std::vector<size_t> tri_indices;
    };

    struct TileBuffer
    {
        size_t width, height;
        std::vector<Tile> tiles;
    };

}

class Rasterizer
{
public:
    Rasterizer() = default;
    void SetRenderState(const RenderState& state);
    void Initialize(std::shared_ptr<Framebuffer> buffer, std::shared_ptr<Viewport> viewport);
    void InitializeTileGrid();

    void Clear(const rasterizer::color4ub& color);
    void Draw(const rasterizer::Mesh& mesh);
    int GetPrimitivesNum() const;

private:
    void ApplyPerspectiveProjection(const rasterizer::Mesh& mesh);
    void ProcessPrimitives(const rasterizer::Mesh& mesh);
    bool InitializeTriangle(rasterizer::RasterTriangle& triangle) const;
    bool ShouldCullTriangle(bool isCCW) const;
    void BinTrianglesToTiles();

    std::shared_ptr<Framebuffer> framebuffer_;
    std::shared_ptr<Viewport> viewport_;

    RenderState state_;

    rasterizer::VertexBuffer vertexBuffer_;
    std::vector<rasterizer::RasterTriangle> triangles_;
    rasterizer::TileBuffer tileBuffer_;
};
