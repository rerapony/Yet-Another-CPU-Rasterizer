#pragma once
#include <cfloat>
#include <fstream>
#include <iostream>
#include <map>

#include "glm/ext/vector_common.hpp"
#include "rasterizer/mesh.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace rasterizer;

namespace utils
{
    inline bool LoadMesh(const std::string& path, Mesh& mesh)
    {
        mesh.positions.clear();
        mesh.fragments.clear();
        mesh.triangles.clear();

        mesh.AABB.minBound = glm::vec3(FLT_MAX);
        mesh.AABB.maxBound = glm::vec3(-FLT_MAX);

        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Error loading mesh from: " << path << std::endl;
            return false;
        }

        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty() || line[0] == '#')
                continue;

            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;
            if (prefix == "v")
            {
                Position p;
                iss >> p.x >> p.y >> p.z;

                mesh.positions.push_back(p);

                mesh.AABB.minBound.x = std::min(mesh.AABB.minBound.x, p.x);
                mesh.AABB.minBound.y = std::min(mesh.AABB.minBound.y, p.y);
                mesh.AABB.minBound.z = std::min(mesh.AABB.minBound.z, p.z);

                mesh.AABB.maxBound.x = std::max(mesh.AABB.maxBound.x, p.x);
                mesh.AABB.maxBound.y = std::max(mesh.AABB.maxBound.y, p.y);
                mesh.AABB.maxBound.z = std::max(mesh.AABB.maxBound.z, p.z);
            }
            else if (prefix == "vt")
            {
                Fragment f;
                iss >> f.uv.x >> f.uv.y;
                f.uv.y = 1.0f - f.uv.y; // due to STB loading images from the top
                mesh.fragments.push_back(f);
            }
            else if (prefix == "f")
            {
                std::vector<VertexIndex> indices;
                std::string vertex_string;
                // vertex data
                while (iss >> vertex_string)
                {
                    VertexIndex index;
                    int v_index, vt_index, n_index;
                    sscanf(vertex_string.c_str(), "%d/%d/%d", &v_index, &vt_index, &n_index);

                    // since obj start indexing from 1
                    index.v = v_index > 0 ? v_index - 1 : mesh.positions.size() + v_index;
                    index.vt = vt_index > 0 ? vt_index - 1 : mesh.fragments.size() + vt_index;

                    indices.push_back(index);
                }

                // we want to only have triangles
                if (indices.size() > 3)
                {
                    std::cerr << "Mesh primitive is not a triangle! Number of vertices received: " << indices.size() << std::endl;
                }

                Triangle triangle;
                triangle.i0 = indices[0];
                triangle.i1 = indices[1];
                triangle.i2 = indices[2];
                mesh.triangles.push_back(triangle);
            }
        }

        return true;
    }

    inline bool LoadTexture(const std::string& path, Texture& texture)
    {
        texture.pixels.clear();

        int channels;
        if (auto data = stbi_load(path.c_str(), &texture.width, &texture.height, &channels, 3))
        {
            int totalPixels = texture.width * texture.height;
            for (int i = 0; i < totalPixels; ++i)
            {
                color4ub pixel_data;
                pixel_data.r = data[i * 3 + 0];
                pixel_data.g = data[i * 3 + 1];
                pixel_data.b = data[i * 3 + 2];
                pixel_data.a = 255;

                texture.pixels.push_back(pixel_data);
            }

            stbi_image_free(data);
        }
        else
        {
            std::cerr << "Error loading texture from: " << path << std::endl;
            return false;
        }

        return true;
    }
}
