#pragma once
#include <string>
#include <vector>

#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"

namespace application
{
    struct EventInfo
    {
        bool shouldQuit = false;
        bool shouldRotate = false;
        float rotXY = 0.0f;
    };

    struct UIInfo
    {
        size_t primitives_num = 0;
    };
}

namespace rasterizer
{
    struct color4ub;
}

class Application
{
public:
    Application(const std::string& app_name, int width, int height, int sidebar_width = 0, SDL_WindowFlags flags = 0);

    Application(Application& other) = delete;
    void operator=(const Application&) = delete;

    void Render(const std::vector<rasterizer::color4ub>& color_buffer, size_t pitch);
    void RenderUIOverlay(const application::UIInfo& ui_info);

    void Update();

    bool Run(application::EventInfo& event_info);
    ~Application();

private:
    void InitSDL(const std::string& app_name, const SDL_WindowFlags& flags);
    void InitImGui() const;

    void ShutdownSDL();
    void ShutdownImGui();

    int width_, height_, sidebar_width_;

    SDL_Event event_{};

    SDL_Window* window_;
    SDL_Renderer* renderer_;
    SDL_Texture* texture_;
};
