#pragma once
#include "glm/gtc/matrix_transform.hpp"

struct Camera
{
    float FOV = glm::radians(45.0f);
    glm::vec3 position = glm::vec3(0.0f);
};