#pragma once
#include <filesystem>
#include <string>
#include <vector>

#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"

namespace config
{
    struct RenderConfig;
    struct MeshConfig;
}

namespace rasterizer
{
    struct color4ub;
}

struct SDL_DialogFileFilter;

namespace application
{
    struct EventInfo
    {
        bool shouldQuit = false;
        bool shouldRotate = false;
        float rotXY = 0.0f;
    };

    struct WindowSizeInfo
    {
        int width, height, sidebar_width;
    };

    struct PerformanceInfo
    {
        size_t primitives_num = 0;
    };
}

class Application
{
public:
    Application(const std::string& app_name, const application::WindowSizeInfo& size_info, std::filesystem::path asset_folder, SDL_WindowFlags flags = 0);
    Application(Application& other) = delete;
    void operator=(const Application&) = delete;

    void Render(const std::vector<rasterizer::color4ub>& color_buffer, size_t pitch);
    void RenderUIOverlay(config::MeshConfig& mesh_config, config::RenderConfig& render_config, const application::PerformanceInfo& performance_info);

    void Update();

    bool Run(application::EventInfo& event_info);
    ~Application();

private:
    void InitSDL(const std::string& app_name, const SDL_WindowFlags& flags);
    void InitImGui() const;

    void ShutdownSDL();
    void ShutdownImGui();

    void RenderOpenFileButton(const std::string& button_name, std::filesystem::path& file_path, const SDL_DialogFileFilter* filters, int filters_num);

    int width_, height_, sidebar_width_;

    std::filesystem::path asset_folder_;

    SDL_Event event_{};
    SDL_Window* window_{};
    SDL_Renderer* renderer_{};
    SDL_Texture* texture_{};
};
