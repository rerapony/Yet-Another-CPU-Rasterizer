#include <filesystem>
#include <string>

#include "config.h"
#include "application/application.h"
#include "assets/utils.h"
#include "glm/detail/func_trigonometric.inl"
#include "rasterizer/rasterizer.h"
#include "rasterizer/viewport.h"

namespace
{
    constexpr int WINDOW_WIDTH = 800;
    constexpr int WINDOW_HEIGHT = 600;
    constexpr int SIDEBAR_WIDTH = 250;
    const std::string WINDOW_TITLE = "CPU Rasterizer";

    rasterizer::color4ub COLOR_DEFAULT = rasterizer::ToColor4UB({1.f, 1.f, 1.f, 1.f});
    std::filesystem::path ASSETS_FOLDER_DEFAULT = "/assets/";
    std::filesystem::path MESH_PATH_DEFAULT = "/assets/cait-sith-low-poly-model/caitsith.obj";
    std::filesystem::path TEXTURE_PATH_DEFAULT  = "/assets/cait-sith-low-poly-model/caitsith.png";

    constexpr float FOV_DEFAULT = 45.0f;
    constexpr float CAMERA_DIST_DEFAULT = 17.5f;
    constexpr float MESH_SIZE = 10.f;

    constexpr float NEAR_PLANE = 0.1f;
    constexpr float FAR_PLANE = 100.f;
}

int main()
{
    using namespace rasterizer;
    using namespace utils;
    using namespace config;

    // project root and executable root may differ
    std::filesystem::path asset_folder = ASSETS_FOLDER_DEFAULT;
    if (!std::filesystem::exists(asset_folder)) {
        asset_folder = PROJECT_ROOT_DIR + asset_folder.string();
    }

    Rasterizer rasterizer;
    int render_width = WINDOW_WIDTH - SIDEBAR_WIDTH;
    auto framebuffer = std::make_shared<Framebuffer>(render_width, WINDOW_HEIGHT);
    auto viewport = std::make_shared<Viewport>(render_width, WINDOW_HEIGHT);
    rasterizer.Initialize(framebuffer, viewport);

    Mesh mesh;
    Texture texture;

    MeshConfig mesh_config;
    std::filesystem::path loaded_mesh_path;
    mesh_config.mesh_path = MESH_PATH_DEFAULT;
    if (!std::filesystem::exists(mesh_config.mesh_path))
    {
        mesh_config.mesh_path = PROJECT_ROOT_DIR + mesh_config.mesh_path.string();
    }

    std::filesystem::path loaded_texture_path;
    mesh_config.texture_path = TEXTURE_PATH_DEFAULT;
    if (!std::filesystem::exists(mesh_config.texture_path))
    {
        mesh_config.texture_path = PROJECT_ROOT_DIR + mesh_config.texture_path.string();
    }

    RenderConfig render_config {Back, CCW};
    RenderState state;
    state.vertexShader.functor = vertex_shader::VertexShaderMVP;
    state.fragmentShader.functor = fragment_shader::FragmentShaderTexture;
    state.nearPlane = NEAR_PLANE;
    state.farPlane = FAR_PLANE;

    CameraConfig camera_config{glm::radians(FOV_DEFAULT), {0, 0, CAMERA_DIST_DEFAULT}};

    Application app(WINDOW_TITLE, {WINDOW_WIDTH, WINDOW_HEIGHT, SIDEBAR_WIDTH}, asset_folder);
    application::EventInfo event_info;
    application::PerformanceInfo performance_info;

    while (true)
    {
        while (app.Run(event_info))
        {
            if (event_info.shouldQuit)
                return 0;
        }

        if (mesh_config.mesh_path != loaded_mesh_path && !mesh_config.mesh_path.empty())
        {
            loaded_mesh_path = mesh_config.mesh_path;
            if (!LoadMesh(std::filesystem::absolute(loaded_mesh_path).string(), mesh))
            {
                return -1;
            }
        }

        if (mesh_config.texture_path != loaded_texture_path && !mesh_config.texture_path.empty())
        {
            loaded_texture_path = mesh_config.texture_path;
            if (!LoadTexture(std::filesystem::absolute(loaded_texture_path).string(), texture))
            {
                return -1;
            }

            state.fragmentShader.uniforms.texture = texture;
        }

        float aspectRatio = viewport->GetAspectRatio();

        glm::mat4 modelMatrix = vertex_shader::NormalizedModelMatrix(mesh.AABB, MESH_SIZE, glm::radians(event_info.rotXY));
        glm::mat4 viewMatrix = vertex_shader::WorldToCameraMatrix(camera_config, glm::vec3(0.0f, 0.0f, 0.0f));
        glm::mat4 projectionMatrix = vertex_shader::PerspectiveMatrix(camera_config, aspectRatio, NEAR_PLANE, FAR_PLANE);

        vertex_shader::VertexUniforms vertexUniforms(projectionMatrix * viewMatrix * modelMatrix);
        state.vertexShader.uniforms = vertexUniforms;
        state.render_config = render_config;

        rasterizer.SetRenderState(state);

        rasterizer.Clear(COLOR_DEFAULT);
        rasterizer.Draw(mesh);

        performance_info.primitives_num = rasterizer.GetPrimitivesNum();

        app.Render(framebuffer->GetColorBuffer(), framebuffer->GetPitch());
        app.RenderUIOverlay(mesh_config, render_config, performance_info);

        app.Update();
    }

    return 0;
}