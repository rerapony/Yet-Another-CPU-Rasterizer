#pragma once
#include <string>
#include <vector>

#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"

namespace rasterizer
{
    struct color4ub;
}

class Application
{
public:
    Application(const std::string& name, int width, int height, SDL_WindowFlags flags = 0);

    Application(Application& other) = delete;
    void operator=(const Application&) = delete;

    void Render(const std::vector<rasterizer::color4ub>& color_buffer) const;
    void PrintDebug(const std::string& text) const;
    void Update() const;

    ~Application();

private:
    // same for the window and the texture resolution
    int width_, height_;

    SDL_Window* window_;
    SDL_Renderer* renderer_;
    SDL_Texture* texture_;
};
