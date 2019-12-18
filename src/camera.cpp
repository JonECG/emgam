#include "camera.h"

#include "glm/ext/matrix_transform.hpp"

using namespace emsg;

Camera emsg::g_camera = {};

void Camera::LookAt(glm::vec3& from, glm::vec3& to, glm::vec3& up)
{
    glm::vec3 diff = to - from;
    glm::vec3 forward = glm::normalize(diff);

    glm::vec3 horizon = glm::normalize(glm::cross(forward, up));
    glm::vec3 vertical = glm::normalize(glm::cross(horizon, forward));

    // Set basis vectors
    view[0] = { horizon, 0 };
    view[1] = { vertical, 0 };
    view[2] = { forward, 0 };
    view[3] = { from, 1 }; // Constant scale translation
}
