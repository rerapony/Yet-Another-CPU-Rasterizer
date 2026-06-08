#include "application.h"

#include <iostream>

#include "types.h"
#include "SDL3/SDL_init.h"

Application::Application(const std::string& name, int width, int height, SDL_WindowFlags flags)
{
    width_ = width;
    height_ = height;

    window_ = SDL_CreateWindow(name.c_str(), width_, height_, flags);
    renderer_ = SDL_CreateRenderer(window_, nullptr);

    SDL_PixelFormat format = SDL_PIXELFORMAT_RGBA32;
    SDL_TextureAccess access = SDL_TEXTUREACCESS_STREAMING; // since we will be writing data from CPU to GPU every frame
    texture_ = SDL_CreateTexture(renderer_, format, access, width_, height_);
}

void Application::Render(const std::vector<rasterizer::color4ub>& color_buffer) const
{
    // total bytes per horizontal row of pixels
    int pitch = width_ * sizeof(rasterizer::color4ub);

    SDL_UpdateTexture(texture_, nullptr, color_buffer.data(), pitch);
    SDL_RenderClear(renderer_);
    SDL_RenderTexture(renderer_, texture_, nullptr, nullptr);
}

void Application::Update() const
{
    SDL_RenderPresent(renderer_);
}

Application::~Application()
{
    if (SDL_WasInit(SDL_INIT_VIDEO))
    {
        if (texture_)
        {
            SDL_DestroyTexture(texture_);
        }

        if (renderer_)
        {
            SDL_DestroyRenderer(renderer_);
        }

        if (window_)
        {
            SDL_DestroyWindow(window_);
        }
    }
}