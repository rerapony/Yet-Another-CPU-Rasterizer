#include <algorithm>

#include "rasterizer.h"
#include "mesh.h"

#include <cmath>
#include <utility>

#include "framebuffer.h"

using namespace rasterizer;

namespace
{
    float EdgeFunction(const glm::vec2& v0, const glm::vec2& v1, const glm::vec2& v2)
    {
        return (v2.x - v0.x) * (v1.y - v0.y) - (v2.y - v0.y) * (v1.x - v0.x);
    }

    bool IsTopLeftEdge(const point &a, const point &b)
    {
        const float dx = b.x - a.x;
        const float dy = b.y - a.y;

        return dy > 0 || (dy == 0 && dx < 0);
    }
}

void VertexBuffer::Clear()
{
    x.clear();
    y.clear();
    z.clear();
    w.clear();
    inv_w.clear();

    u.clear();
    v.clear();
}

void VertexBuffer::Resize(const size_t size)
{
    x.resize(size);
    y.resize(size);
    z.resize(size);
    w.resize(size);
    inv_w.resize(size);

    u.resize(size);
    v.resize(size);
}

void Rasterizer::SetRenderState(const RenderState& state)
{
    state_ = state;
}

void Rasterizer::Initialize(std::shared_ptr<Framebuffer> buffer, std::shared_ptr<Viewport> viewport)
{
    framebuffer_ = std::move(buffer);
    viewport_ = std::move(viewport);

    InitializeTileGrid();
}

void Rasterizer::InitializeTileGrid()
{
    tileBuffer_.width = std::ceil(static_cast<float>(viewport_->GetWidth()) / TILE_SIZE);
    tileBuffer_.height = std::ceil(static_cast<float>(viewport_->GetHeight()) / TILE_SIZE);

    tileBuffer_.tiles.reserve( tileBuffer_.width * tileBuffer_.height);

    for (int j = 0; j < tileBuffer_.height; ++j)
    {
        for (int i = 0; i < tileBuffer_.width; ++i)
        {
            Tile tile;
            tile.startX = i * TILE_SIZE;
            tile.startY = j * TILE_SIZE;
            tile.endX = std::min<int>(tile.startX + TILE_SIZE - 1, viewport_->GetWidth() - 1);
            tile.endY = std::min<int>(tile.startY + TILE_SIZE - 1, viewport_->GetHeight() - 1);
            tileBuffer_.tiles.push_back(tile);
        }
    }
}

void Rasterizer::Clear(const color4ub& color)
{
    framebuffer_->Clear(color);
    vertexBuffer_.Clear();
    triangles_.clear();

    for (auto& tile : tileBuffer_.tiles)
    {
        tile.tri_indices.clear();
    }
}

void Rasterizer::Draw(const Mesh& mesh)
{
    // Step 1: Vertex Shader & Perspective Projection
    ApplyPerspectiveProjection(mesh);

    // Step 2: Primitive assembly
    ProcessPrimitives(mesh);

    // Step 3: Tile binning
    BinTrianglesToTiles();

    // Step 4: Tile rasterization
    for (const auto& tile : tileBuffer_.tiles)
    {
        for (size_t triangle_index : tile.tri_indices)
        {
            RasterTriangle triangle = triangles_[triangle_index];
            size_t i0 = triangle.i0, i1 = triangle.i1, i2 = triangle.i2;
            point p0 = AsPoint(vertexBuffer_.x[i0], vertexBuffer_.y[i0], vertexBuffer_.z[i0]);
            point p1 = AsPoint(vertexBuffer_.x[i1], vertexBuffer_.y[i1], vertexBuffer_.z[i1]);
            point p2 = AsPoint(vertexBuffer_.x[i2], vertexBuffer_.y[i2], vertexBuffer_.z[i2]);

            int minX = std::max<int>(tile.startX, triangle.AABB.minBound.x);
            int minY = std::max<int>(tile.startY, triangle.AABB.minBound.y);
            int maxX = std::min<int>(tile.endX, triangle.AABB.maxBound.x);
            int maxY = std::min<int>(tile.endY, triangle.AABB.maxBound.y);

            // int minX = tile.startX;
            // int minY = tile.startY;
            // int maxX = tile.endX;
            // int maxY = tile.endY;

            for (int x = minX; x <= maxX; ++x)
            {
                for (int y = minY; y <= maxY; ++y)
                {
                    point p = AsPoint({x + 0.5f, y + 0.5f, 0.f});

                    float e0 = EdgeFunction(p1, p2, p);
                    float e1 = EdgeFunction(p2, p0, p);
                    float e2 = EdgeFunction(p0, p1, p);

                    if (e0 >= 0.f && e1 >= 0.f && e2 >= 0.f)
                    {
                        e0 /= triangle.e;
                        e1 /= triangle.e;
                        e2 /= triangle.e;

                        float Zndc = e0 * p0.z + e1 * p1.z + e2 * p2.z;
                        if (Zndc < framebuffer_->GetDepth(x, y))
                        {
                            framebuffer_->SetDepth(x, y, Zndc);
                            fragment_shader::Fragment f;
                            f.depth = Zndc;

                            // perspective correct
                            float inv_w = e0 * vertexBuffer_.inv_w[i0] + e1 * vertexBuffer_.inv_w[i1] + e2 * vertexBuffer_.inv_w[i2];
                            float w = 1.f/inv_w;

                            f.u = (e0 * vertexBuffer_.u[i0] + e1 * vertexBuffer_.u[i1] + e2 * vertexBuffer_.u[i2]) * w;
                            f.v = (e0 * vertexBuffer_.v[i0] + e1 * vertexBuffer_.v[i1] + e2 * vertexBuffer_.v[i2]) * w;

                            framebuffer_->SetPixel(x, y, state_.fragmentShader(f));
                        }
                    }
                }
            }
        }
    }
}

int Rasterizer::GetPrimitivesNum() const
{
    return triangles_.size();
}

void Rasterizer::ApplyPerspectiveProjection(const Mesh& mesh)
{
    const size_t vertices_num = mesh.v_indices.size();
    vertexBuffer_.Resize(vertices_num);

    // TODO: parallel
    for (size_t i = 0; i < vertices_num; ++i) {
        const size_t vertex_index = mesh.v_indices[i];
        const size_t texture_index = mesh.vt_indices[i];

        // Step 1.1: Vertex shader
        const point transformed = state_.vertexShader(AsPoint(mesh.x[vertex_index], mesh.y[vertex_index], mesh.z[vertex_index]));
        vertexBuffer_.x[i] = transformed.x;
        vertexBuffer_.y[i] = transformed.y;
        vertexBuffer_.z[i] = transformed.z;
        vertexBuffer_.w[i] = transformed.w;

        // Step 1.2: Perspective divide
        vertexBuffer_.inv_w[i] = 1.0f/vertexBuffer_.w[i];

        vertexBuffer_.x[i] *= vertexBuffer_.inv_w[i];
        vertexBuffer_.y[i] *= vertexBuffer_.inv_w[i];
        vertexBuffer_.z[i] *= vertexBuffer_.inv_w[i];

        // Step 1.3: NDC to Viewport transform
        vertexBuffer_.x[i] = viewport_->GetWidth() * (0.5f + 0.5f * vertexBuffer_.x[i]);
        vertexBuffer_.y[i] = viewport_->GetHeight() * (0.5f - 0.5f * vertexBuffer_.y[i]);

        // Step 1.4: Perspective correct interpolation
        vertexBuffer_.u[i] = mesh.u[texture_index] * vertexBuffer_.inv_w[i];
        vertexBuffer_.v[i] = mesh.v[texture_index] * vertexBuffer_.inv_w[i];
    }
}

void Rasterizer::ProcessPrimitives(const Mesh& mesh)
{
    //TODO: parallel
    for (size_t i = 0; i < mesh.primitives_num; ++i)
    {
        const size_t i0 = i * 3, i1 = i * 3 + 1, i2 = i * 3 + 2;

        // Step 2.1: Simplified clipping
        if (vertexBuffer_.w[i0] <= state_.nearPlane || vertexBuffer_.w[i1] <= state_.nearPlane || vertexBuffer_.w[i2] <= state_.nearPlane)
            continue;

        RasterTriangle triangle;
        triangle.i0 = i0;
        triangle.i1 = i1;
        triangle.i2 = i2;

        if (!InitializeTriangle(triangle)) // triangle is not culled
            continue;

        triangles_.push_back(triangle);
    }
}

bool Rasterizer::InitializeTriangle(RasterTriangle& triangle) const
{
    glm::vec2 v0 = glm::vec2(vertexBuffer_.x[triangle.i0], vertexBuffer_.y[triangle.i0]);
    glm::vec2 v1 = glm::vec2(vertexBuffer_.x[triangle.i1], vertexBuffer_.y[triangle.i1]);
    glm::vec2 v2 = glm::vec2(vertexBuffer_.x[triangle.i2], vertexBuffer_.y[triangle.i2]);

    float e = EdgeFunction(v0, v1, v2); // area of the triangle multiplied by 2
    const bool isCCW = e > 0.f;
    if (ShouldCullTriangle(isCCW))
        return false;

    // adjusting unculled triangles to be CCW
    if (!isCCW)
    {
        std::swap(triangle.i1, triangle.i2);
        e = -e;
    }

    float xmin = glm::min(v0.x, glm::min(v1.x, v2.x));
    float xmax = glm::max(v0.x, glm::max(v1.x, v2.x));
    float ymin = glm::min(v0.y, glm::min(v1.y, v2.y));
    float ymax = glm::max(v0.y, glm::max(v1.y, v2.y));

    xmin = std::max<float>(0.f, xmin);
    xmax = std::min<float>(viewport_->GetWidth()-1, xmax);
    ymin = std::max<float>(0.f, ymin);
    ymax = std::min<float>(viewport_->GetHeight()-1, ymax);

    triangle.e = e;
    triangle.AABB.minBound.x = xmin;
    triangle.AABB.minBound.y = ymin;
    triangle.AABB.maxBound.x = xmax;
    triangle.AABB.maxBound.y = ymax;
    return true;
}

bool Rasterizer::ShouldCullTriangle(const bool isCCW) const
{
    const bool isFrontFace = state_.windingOrder == CCW && isCCW || state_.windingOrder == CW && !isCCW;
    switch (state_.cullMode)
    {
    case Front:
        return isFrontFace;
    case Back:
        return !isFrontFace;
    default:
        return false;
    }
}

void Rasterizer::BinTrianglesToTiles()
{
    // TODO: parallel
    for (size_t i = 0; i < triangles_.size(); ++i)
    {
        const RasterTriangle& triangle = triangles_[i];
        const size_t startTileX = std::floor(triangle.AABB.minBound.x / TILE_SIZE);
        const size_t startTileY = std::floor(triangle.AABB.minBound.y / TILE_SIZE);
        const size_t endTileX = std::floor(triangle.AABB.maxBound.x / TILE_SIZE);
        const size_t endTileY = std::floor(triangle.AABB.maxBound.y / TILE_SIZE);

        for (size_t ix = startTileX; ix <= endTileX; ++ix)
        {
            for (size_t iy = startTileY; iy <= endTileY; ++iy)
            {
                const size_t tile_index = iy * tileBuffer_.width + ix;
                tileBuffer_.tiles[tile_index].tri_indices.push_back(i);
            }
        }
    }
}