#pragma once
#include "fragment_shader.h"
#include "vertex_shader.h"

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

// same as PSO
struct RenderState
{
    CullMode cullMode = None;
    WindingOrder windingOrder = CCW;

    vertex_shader::VertexShader vertexShader;
    fragment_shader::FragmentShader fragmentShader;

    float nearPlane;
    float farPlane;
};
