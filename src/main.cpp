#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>

#include "SDL3/SDL.h"

#include "application/application.h"
#include "platform/utils.h"
#include "rasterizer/camera.h"
#include "rasterizer/rasterizer.h"
#include "rasterizer/viewport.h"

namespace
{
    constexpr size_t WINDOW_WIDTH = 800;
    constexpr size_t WINDOW_HEIGHT = 600;
    const std::string WINDOW_TITLE = "CPU Rasterizer";

    color4ub COLOR_DEFAULT = ToColor4UB({1.f, 1.f, 1.f, 1.f});
    std::filesystem::path MESH_PATH = "/obj/cait-sith-low-poly-model/1.obj";
    std::filesystem::path TEXTURE_PATH  = "/obj/cait-sith-low-poly-model/caitsith.png";

    constexpr float CAMERA_DIST = 20.f;
    constexpr float MESH_SIZE = 10.f;
    constexpr float MOUSE_SENSITIVITY = 0.1f;

    constexpr float NEAR_PLANE = 0.1f;
    constexpr float FAR_PLANE = 100.f;
}

int main()
{
    using namespace rasterizer;
    using namespace utils;

    SDL_Init(SDL_INIT_VIDEO);

    float rotXY = 0;

    Application app(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);

    bool bIsRunning = true;
    bool bIsRotating = false;
    SDL_Event event;

    Camera camera;
    camera.FOV = glm::radians(45.f);
    camera.position = glm::vec3(0.0f, 0.0f, CAMERA_DIST);

    Framebuffer framebuffer(WINDOW_WIDTH, WINDOW_HEIGHT);
    Rasterizer rasterizer(&framebuffer);

    Viewport viewport(0.f, WINDOW_WIDTH, 0.f, WINDOW_HEIGHT);
    rasterizer.SetViewport(viewport);

    // project root and executable root may differ
    std::filesystem::path meshPath = MESH_PATH;
    if (!std::filesystem::exists(meshPath)) {
        meshPath = PROJECT_ROOT_DIR + meshPath.string();
    }

    Mesh mesh;
    if (!LoadMesh(absolute(meshPath).string(), mesh))
    {
        SDL_Quit();
        return -1;
    }

    std::filesystem::path texturePath = TEXTURE_PATH;
    if (!std::filesystem::exists(texturePath)) {
        texturePath = PROJECT_ROOT_DIR + texturePath.string();
    }

    Texture texture;
    LoadTexture(absolute(texturePath).string(), texture);

    RenderState settings;
    settings.vertexShader.functor = vertex_shader::VertexShaderMVP;
    settings.fragmentShader.functor = fragment_shader::FragmentShaderDepthMap;
    settings.fragmentShader.uniforms.texture = texture;

    using clock = std::chrono::high_resolution_clock;
    auto last_frame_start = clock::now();

    while (bIsRunning)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                {
                    bIsRunning = false;
                    break;
                }
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                {
                    if (event.button.button == SDL_BUTTON_RIGHT)
                    {
                        bIsRotating = true;
                    }
                    break;
                }
            case SDL_EVENT_MOUSE_BUTTON_UP:
                {
                    if (event.button.button == SDL_BUTTON_RIGHT)
                    {
                        bIsRotating = false;
                    }
                    break;
                }
            case SDL_EVENT_MOUSE_MOTION:
                {
                    if (bIsRotating)
                    {
                        // how much did the mouse move?
                        float dx = event.motion.xrel;
                        //float dy = event.motion.y - mouse_y;
                        rotXY += dx;
                        rotXY = glm::clamp(rotXY, -90.0f, 90.0f);
                    }
                    break;
                }
            default: ;
            }

            auto current_frame = clock::now();
            const float dt = std::chrono::duration_cast<std::chrono::duration<float>>(current_frame - last_frame_start).count();
            last_frame_start = current_frame;

            std::cout << dt << std::endl;

            if (!bIsRunning)
                break;

            float aspectRatio = viewport.GetAspectRatio();

            glm::mat4 modelMatrix = vertex_shader::NormalizedModelMatrix(mesh, MESH_SIZE, glm::radians(rotXY));
            glm::mat4 viewMatrix = vertex_shader::WorldToCameraMatrix(camera, glm::vec3(0.0f, 0.0f, 0.0f));
            glm::mat4 projectionMatrix = vertex_shader::PerspectiveMatrix(camera, aspectRatio, NEAR_PLANE, FAR_PLANE);

            vertex_shader::VertexUniforms vertexUniforms(projectionMatrix * viewMatrix * modelMatrix);
            settings.vertexShader.uniforms = vertexUniforms;

            float meshRadius = glm::distance(modelMatrix*AsPoint(mesh.AABB.minBound), modelMatrix*AsPoint(mesh.AABB.maxBound)) * 0.5;
            float minDepth = CAMERA_DIST - meshRadius;
            float maxDepth = CAMERA_DIST + meshRadius;

            settings.fragmentShader.uniforms.nearPlane = NEAR_PLANE;
            settings.fragmentShader.uniforms.farPlane = FAR_PLANE;
            settings.fragmentShader.uniforms.minDepth = minDepth;
            settings.fragmentShader.uniforms.maxDepth = maxDepth;

            rasterizer.SetRenderingSettings(settings);

            rasterizer.Clear(COLOR_DEFAULT);
            rasterizer.Draw(mesh);
            app.Render(framebuffer.GetColorBuffer());
            app.Update();
        }
    }

    SDL_Quit();

    return 0;
}