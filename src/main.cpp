#include <filesystem>
#include <string>

#include "application/application.h"
#include "assets/utils.h"
#include "rasterizer/camera.h"
#include "rasterizer/rasterizer.h"
#include "rasterizer/viewport.h"

namespace
{
    constexpr int WINDOW_WIDTH = 800;
    constexpr int WINDOW_HEIGHT = 600;
    constexpr int SIDEBAR_WIDTH = 250;
    const std::string WINDOW_TITLE = "CPU Rasterizer";

    color4ub COLOR_DEFAULT = ToColor4UB({1.f, 1.f, 1.f, 1.f});
    std::filesystem::path MESH_PATH = "/obj/cait-sith-low-poly-model/1.obj";
    std::filesystem::path TEXTURE_PATH  = "/obj/cait-sith-low-poly-model/caitsith.png";

    constexpr float CAMERA_DIST = 15.f;
    constexpr float MESH_SIZE = 10.f;
    constexpr float MOUSE_SENSITIVITY = 0.1f;

    constexpr float NEAR_PLANE = 0.1f;
    constexpr float FAR_PLANE = 100.f;
}

int main()
{
    using namespace rasterizer;
    using namespace utils;

    Application app(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, SIDEBAR_WIDTH);

    application::EventInfo eventInfo;
    application::UIInfo uiInfo;

    Camera camera;
    camera.FOV = glm::radians(45.f);
    camera.position = glm::vec3(0.0f, 0.0f, CAMERA_DIST);

    Rasterizer rasterizer;
    int render_width = WINDOW_WIDTH - SIDEBAR_WIDTH;
    auto framebuffer = std::make_shared<Framebuffer>(render_width, WINDOW_HEIGHT);
    auto viewport = std::make_shared<Viewport>(render_width, WINDOW_HEIGHT);
    rasterizer.Initialize(framebuffer, viewport);

    // project root and executable root may differ
    std::filesystem::path meshPath = MESH_PATH;
    if (!std::filesystem::exists(meshPath)) {
        meshPath = PROJECT_ROOT_DIR + meshPath.string();
    }

    Mesh mesh;
    if (!LoadMesh(absolute(meshPath).string(), mesh))
    {
        return -1;
    }

    std::filesystem::path texturePath = TEXTURE_PATH;
    if (!std::filesystem::exists(texturePath)) {
        texturePath = PROJECT_ROOT_DIR + texturePath.string();
    }

    Texture texture;
    LoadTexture(absolute(texturePath).string(), texture);

    RenderState state;
    state.vertexShader.functor = vertex_shader::VertexShaderMVP;
    state.fragmentShader.functor = fragment_shader::FragmentShaderTexture;
    state.fragmentShader.uniforms.texture = texture;
    state.nearPlane = NEAR_PLANE;
    state.farPlane = FAR_PLANE;
    state.cullMode = Back;

    while (true)
    {
        while (app.Run(eventInfo))
        {
            if (eventInfo.shouldQuit)
                return 0;
        }

        float aspectRatio = viewport->GetAspectRatio();

        glm::mat4 modelMatrix = vertex_shader::NormalizedModelMatrix(mesh.AABB, MESH_SIZE, glm::radians(eventInfo.rotXY));
        glm::mat4 viewMatrix = vertex_shader::WorldToCameraMatrix(camera, glm::vec3(0.0f, 0.0f, 0.0f));
        glm::mat4 projectionMatrix = vertex_shader::PerspectiveMatrix(camera, aspectRatio, NEAR_PLANE, FAR_PLANE);

        vertex_shader::VertexUniforms vertexUniforms(projectionMatrix * viewMatrix * modelMatrix);
        state.vertexShader.uniforms = vertexUniforms;

        rasterizer.SetRenderState(state);

        rasterizer.Clear(COLOR_DEFAULT);
        rasterizer.Draw(mesh);

        uiInfo.primitives_num = rasterizer.GetPrimitivesNum();

        app.Render(framebuffer->GetColorBuffer(), framebuffer->GetPitch());
        app.RenderUIOverlay(uiInfo);

        app.Update();
    }

    return 0;
}