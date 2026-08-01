#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

#include "glm/ext/vector_common.hpp"
#include "rasterizer/mesh.h"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

namespace utils
{
    using namespace rasterizer;

    inline bool LoadMesh(const std::string& path, Mesh& mesh)
    {
        mesh.Reset();

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
                float x, y, z;
                iss >> x >> y >> z;

                mesh.x.push_back(x);
                mesh.y.push_back(y);
                mesh.z.push_back(z);

                mesh.AABB.minBound.x = std::min(mesh.AABB.minBound.x, x);
                mesh.AABB.minBound.y = std::min(mesh.AABB.minBound.y, y);
                mesh.AABB.minBound.z = std::min(mesh.AABB.minBound.z, z);

                mesh.AABB.maxBound.x = std::max(mesh.AABB.maxBound.x, x);
                mesh.AABB.maxBound.y = std::max(mesh.AABB.maxBound.y, y);
                mesh.AABB.maxBound.z = std::max(mesh.AABB.maxBound.z, z);
            }
            else if (prefix == "vt")
            {
                float u, v;
                iss >> u >> v;
                v = 1.0f - v; // due to STB loading images from the top
                mesh.u.push_back(u);
                mesh.v.push_back(v);
            }
            else if (prefix == "f")
            {
                std::string vertex_string;
                std::vector<int> vertex_indices;
                std::vector<int> texture_indices;
                while (iss >> vertex_string)
                {
                    int v_index, vt_index, n_index;
                    sscanf(vertex_string.c_str(), "%d/%d/%d", &v_index, &vt_index, &n_index);

                    // since assets start indexing from 1
                    v_index = v_index > 0 ? v_index - 1 : mesh.x.size() + v_index;
                    vt_index = vt_index > 0 ? vt_index - 1 : mesh.x.size() + vt_index;

                    vertex_indices.push_back(v_index);
                    texture_indices.push_back(vt_index);
                }

                if (vertex_indices.size() < 3 || texture_indices.size() < 3)
                {
                    std::cerr << "Error reading primitive for a mesh!!! Expected 3 vertices, received: " << vertex_indices.size() << " Skipping..." << std::endl;
                    continue;
                }

                if (vertex_indices.size() > 3 || texture_indices.size() > 3)
                {
                    std::cerr << "Mesh primitive is not a triangle! Only first 3 vertices will be accepted. Number of vertices received: " << vertex_indices.size() << std::endl;
                }

                mesh.v_indices.insert(mesh.v_indices.end(), vertex_indices.begin(), vertex_indices.begin() + 3);
                mesh.vt_indices.insert(mesh.vt_indices.end(), texture_indices.begin(), texture_indices.begin() + 3);
            }
        }

        if (mesh.v_indices.size() != mesh.vt_indices.size())
        {
            std::cerr << "Vertex indices and texture indices do not match!" << std::endl;
            return false;
        }

        if (mesh.vt_indices.size() % 3 != 0)
        {
            std::cerr << "Number of vertices must be a multiple of 3!" << std::endl;
            return false;
        }

        mesh.primitives_num = mesh.v_indices.size()/3;
        return true;
    }

    inline bool LoadTexture(const std::string& path, Texture& texture)
    {
        texture.pixels.clear();

        int channels;
        if (const auto data = stbi_load(path.c_str(), &texture.width, &texture.height, &channels, 3))
        {
            int totalPixels = texture.width * texture.height;
            for (int i = 0; i < totalPixels; ++i)
            {
                color4ub pixel_data{};
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
