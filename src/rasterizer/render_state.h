#pragma once
#include "fragment_shader.h"
#include "vertex_shader.h"
#include "config.h"

// same as PSO
struct RenderState
{
    config::RenderConfig render_config;

    vertex_shader::VertexShader vertexShader;
    fragment_shader::FragmentShader fragmentShader;

    float nearPlane = 0.f;
    float farPlane = 0.f;
};
