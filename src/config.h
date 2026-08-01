#pragma once
#include <filesystem>

#include "glm/vec3.hpp"

namespace config
{
    enum CullMode
    {
        None,
        Front,
        Back
    };

    enum WindingOrder
    {
        CW,
        CCW
    };

    struct MeshConfig
    {
        std::filesystem::path mesh_path;
        std::filesystem::path texture_path;
    };

    struct CameraConfig
    {
        float FOV = 45.0f;
        glm::vec3 position;
    };

    struct RenderConfig
    {
        CullMode cullMode = Back;
        WindingOrder windingOrder = CCW;
    };
}
