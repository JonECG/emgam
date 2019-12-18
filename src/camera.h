#pragma once

#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"

namespace emsg
{
    struct Camera
    {
        glm::mat4 view;
        glm::mat4 projection;
        void LookAt(glm::vec3& from, glm::vec3& to, glm::vec3& up);
    };

    extern Camera g_camera;
}
