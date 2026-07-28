#include "application.h"

#include <iostream>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "imgui_internal.h"
#include "types.h"
#include "SDL3/SDL_init.h"

Application::Application(const std::string& app_name, int width, int height, int sidebar_width, SDL_WindowFlags flags)
{
    width_ = width;
    height_ = height;
    sidebar_width_ = sidebar_width;

    InitSDL(app_name, flags);
    InitImGui();
}

Application::~Application()
{
    ShutdownImGui();
    ShutdownSDL();
}

void Application::InitSDL(const std::string& app_name, const SDL_WindowFlags& flags)
{
    SDL_Init(SDL_INIT_VIDEO);

    window_ = SDL_CreateWindow(app_name.c_str(), width_, height_, flags);
    renderer_ = SDL_CreateRenderer(window_, nullptr);

    SDL_PixelFormat format = SDL_PIXELFORMAT_RGBA32;
    SDL_TextureAccess access = SDL_TEXTUREACCESS_STREAMING; // since we will be writing data from CPU to GPU every frame
    texture_ = SDL_CreateTexture(renderer_, format, access, width_, height_);
}

void Application::InitImGui() const
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsLight();

    ImGui_ImplSDL3_InitForSDLRenderer(window_, renderer_);
    ImGui_ImplSDLRenderer3_Init(renderer_);
}

void Application::ShutdownSDL()
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

    SDL_Quit();
}

void Application::ShutdownImGui()
{
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void Application::Render(const std::vector<rasterizer::color4ub>& color_buffer, size_t pitch)
{
    SDL_UpdateTexture(texture_, nullptr, color_buffer.data(), pitch);
    SDL_RenderClear(renderer_);
    SDL_RenderTexture(renderer_, texture_, nullptr, nullptr);
}

void Application::RenderUIOverlay(const application::UIInfo& ui_info)
{
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(static_cast<float>(width_ - sidebar_width_), 0.f));
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(sidebar_width_), static_cast<float>(height_)));

    // sidebar flags
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize
                           | ImGuiWindowFlags_NoMove
                           | ImGuiWindowFlags_NoCollapse
                           | ImGuiWindowFlags_NoBringToFrontOnFocus;


    
    // UI Overlay Begin
    ImGui::Begin("Rasterizer Performance", nullptr, flags);
    ImGui::Separator();
    ImGui::Text("Primitives: %zu", ui_info.primitives_num);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::End();
    // UI Overlay End

    ImGui::Render();
}

void Application::Update()
{
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer_);
    SDL_RenderPresent(renderer_);
}

bool Application::Run(application::EventInfo& event_info)
{
    const bool result = SDL_PollEvent(&event_);
    ImGui_ImplSDL3_ProcessEvent(&event_);

    switch (event_.type)
    {
    case SDL_EVENT_QUIT:
        {
            event_info.shouldQuit = true;
            break;
        }
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        {
            if (event_.button.button == SDL_BUTTON_RIGHT)
            {
                event_info.shouldRotate = true;
            }
            break;
        }
    case SDL_EVENT_MOUSE_BUTTON_UP:
        {
            if (event_.button.button == SDL_BUTTON_RIGHT)
            {
                event_info.shouldRotate = false;
            }
            break;
        }
    case SDL_EVENT_MOUSE_MOTION:
        {
            if (event_info.shouldRotate)
            {
                // how much did the mouse move?
                const float dx = event_.motion.xrel;
                //float dy = event.motion.y - mouse_y;
                event_info.rotXY += dx;
                event_info.rotXY = event_info.rotXY > 360.0f ? .0f : event_info.rotXY;
                event_info.rotXY = event_info.rotXY < -360.0 ? .0f : event_info.rotXY;
            }
            break;
        }
    default: ;
    }

    return result;
}