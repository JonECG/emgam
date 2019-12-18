#include "model.h"

using namespace emsg;

namespace
{
    glm::vec3 up(0, 1, 0);
    glm::vec3 right(1, 0, 0);
    glm::vec3 forward(0, 0, 1);

    glm::vec3 down = -up;
    glm::vec3 left = -right;
    glm::vec3 back = -forward;
}

/*
Model Model::CreateBox(float dim)
{
    Model m;
    m.vertices = { {0.0f, 0.5f, 0.0f}, {-0.5f, -0.5f, 0.0f},{ 0.5f, -0.5f, 0.0f } };
    m.indices = { 0, 1, 2 };
    return m;
}
*/

Model Model::CreateBox(float dim)
{
    Model m;
    glm::vec3* faceDirs[] = { &up, &right, &forward, &down, &left, &back };
    //glm::vec3* xDir[] = { &right, &right, &forward, &down, &left, &back };
    unsigned short idx = 0;
    for (int i = 0; i < 6; ++i)
    {
        glm::vec3& faceDir = *faceDirs[i];
        glm::vec3 xDir = { faceDir.y, faceDir.z, faceDir.x };
        glm::vec3 yDir = { faceDir.z, faceDir.x, faceDir.y };

        m.vertices.emplace_back(faceDir + yDir - xDir); m.indices.emplace_back(idx++);
        m.vertices.emplace_back(faceDir + yDir + xDir); m.indices.emplace_back(idx++);
        m.vertices.emplace_back(faceDir + xDir - yDir); m.indices.emplace_back(idx++);

        m.vertices.emplace_back(faceDir + xDir - yDir); m.indices.emplace_back(idx++);
        m.vertices.emplace_back(faceDir - xDir - yDir); m.indices.emplace_back(idx++);
        m.vertices.emplace_back(faceDir + yDir - xDir); m.indices.emplace_back(idx++);
    }

    return m;
}
