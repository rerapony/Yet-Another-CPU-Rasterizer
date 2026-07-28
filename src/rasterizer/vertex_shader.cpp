#include "vertex_shader.h"

#include "camera.h"

using namespace rasterizer;

namespace vertex_shader
{
    glm::mat4 NormalizedModelMatrix(const BoundingBox& aabb, float targetSize, float rotY)
    {
        if (aabb.minBound == aabb.maxBound)
            return {1.0f};

        glm::vec3 meshCenter = (aabb.minBound + aabb.maxBound) * 0.5f;
        glm::vec3 meshSpread = aabb.maxBound - aabb.minBound; // vec
        float maxExtent = std::max({meshSpread.x, meshSpread.y, meshSpread.z});
        float scale = targetSize / maxExtent;

        glm::mat4 scale_mat = glm::scale(glm::mat4(1.0f),glm::vec3(scale, scale, scale));
        glm::mat4 rotation_mat = glm::rotate(glm::mat4(1.0f), rotY, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 translation_mat = glm::translate(glm::mat4(1.0f), -meshCenter);

        return scale_mat * rotation_mat * translation_mat;
    }

    glm::mat4 WorldToCameraMatrix(const Camera& camera, const glm::vec3& targetPosition, const glm::vec3& upVector)
    {
        // camera rotation
        //
        glm::vec3 forward = glm::normalize(targetPosition - camera.position); // forward vector
        glm::vec3 right = glm::normalize(glm::cross(forward, upVector));
        glm::vec3 up = glm::cross(right, forward); // guaranteed to be normalized

        glm::mat4 result(1.0f);

        result[0][0] = right.x;
        result[1][0] = right.y;
        result[2][0] = right.z;

        result[0][1] = up.x;
        result[1][1] = up.y;
        result[2][1] = up.z;

        result[0][2] = -forward.x;
        result[1][2] = -forward.y;
        result[2][2] = -forward.z;

        // translation
        result = translate(result, {-camera.position.x, -camera.position.y, -camera.position.z});

        return result;
    }

    glm::mat4 PerspectiveMatrix(const Camera& camera, float aspectRatio, float near, float far)
    {
        glm::mat4 result(0.0f);

        float top = glm::tan(camera.FOV * 0.5f) * near;
        float right = top * aspectRatio;

        // X and Y scaling
        result[0][0] = near/right;
        result[1][1] = near/top;

        // Remapping Z to [-1, 1]
        result[2][2] = -(far+near)/(far-near);

        // W for the perspective divide
        result[2][3] = -1.0f;

        // Z translation
        result[3][2] = -(2.0f * far * near) / (far - near);

        return result;
    }
}
