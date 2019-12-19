#include "model.h"

#if EMSCRIPTEN
    #include <GLES2/gl2.h>
    #include <EGL/egl.h>
    #include <emscripten.h>
#else
    #include <GL/glew.h>
    #include <GLFW/glfw3.h>
#endif

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

void Model::BakeBuffers()
{
    // VBO
    {
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            vertices.size() * sizeof(decltype(vertices)::value_type),
            vertices.data(),
            GL_STATIC_DRAW);
    }

    // IBO
    {
        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            indices.size() * sizeof(decltype(indices)::value_type),
            indices.data(),
            GL_STATIC_DRAW);
    }
}
