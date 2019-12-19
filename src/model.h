#pragma once

#include "glm/vec3.hpp"

#include <vector>

namespace emsg
{
    struct Model
    {
        std::vector<glm::vec3> vertices;
        std::vector<unsigned short> indices;

        unsigned int vbo, ibo;

        static Model CreateBox(float dim);
        void BakeBuffers();
    };
}
