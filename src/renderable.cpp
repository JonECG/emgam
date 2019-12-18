#include "camera.h"
#include "renderable.h"

#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <unordered_map>

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
    std::array<Shader, 20> s_shaders;
    int s_currentShader = 0;
    std::unordered_map<std::string, Shader*> s_shaderMap;

    GLuint LoadShader(GLenum type, const char* shaderSrc)
    {
        GLuint shader;
        GLint compiled;

        // Create the shader object
        shader = glCreateShader(type);

        if (shader == 0)
            return 0;

        // Load the shader source
        glShaderSource(shader, 1, &shaderSrc, NULL);

        // Compile the shader
        glCompileShader(shader);

        // Check the compile status
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

        if (!compiled)
        {
            GLint infoLen = 0;

            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

            if (infoLen > 1)
            {
                char* infoLog = static_cast<char*>(malloc(sizeof(char) * infoLen));

                glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
                //esLogMessage ( "Error compiling shader:\n%s\n", infoLog );            

                free(infoLog);
            }

            glDeleteShader(shader);
            return 0;
        }

        return shader;

    }
}

Shader* emsg::Shader::Get(const std::string& name)
{
    auto iter = s_shaderMap.find(name);
    if (iter != s_shaderMap.end()) return iter->second;

    return nullptr;
}

Shader* emsg::Shader::Make(
    const std::string& name, const std::string& vert, const std::string& frag, Shader::Properties&& props)
{
    auto get = Get(name);
    if (get) return get;

    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint programObject;
    GLint linked;

    // Load the vertex/fragment shaders
    vertexShader = LoadShader(GL_VERTEX_SHADER, vert.c_str());
    fragmentShader = LoadShader(GL_FRAGMENT_SHADER, frag.c_str());

    // Create the program object
    programObject = glCreateProgram();

    if (programObject == 0)
        return nullptr;

    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);

    // Bind attributes
    int attribute = 0;
    glBindAttribLocation(programObject, attribute++, "vTransform");
    for (const auto& iter : props)
    {
        glBindAttribLocation(programObject, attribute++, iter.first.c_str());
    }

    // Link the program
    glLinkProgram(programObject);

    // Check the link status
    glGetProgramiv(programObject, GL_LINK_STATUS, &linked);

    if (!linked)
    {
        GLint infoLen = 0;

        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 1)
        {
            char* infoLog = static_cast<char*>(malloc(sizeof(char) * infoLen));

            glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
            //esLogMessage ( "Error linking program:\n%s\n", infoLog );            

            free(infoLog);
        }

        glDeleteProgram(programObject);
        return nullptr;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    Shader& result = s_shaders[s_currentShader++];
    result.defaultProperties = std::move(props);
    result.opaque = reinterpret_cast<void*>(programObject);
    return &result;
}

Shader::Instance emsg::Shader::Instantiate() const
{
    Shader::Instance inst;
    inst.shader = this;
    inst.properties = defaultProperties;
    return inst;
}

void emsg::Renderable::Clear(const Context& ctx)
{
    glViewport(0, 0, ctx.width, ctx.height);
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void emsg::Renderable::Draw() const
{
    // VBO
    GLuint vbo;
    {
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            model->vertices.size() * sizeof(decltype(model->vertices)::value_type),
            model->vertices.data(),
            GL_STATIC_DRAW);
    }

    // IBO
    GLuint ibo;
    {
        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            model->indices.size() * sizeof(decltype(model->indices)::value_type),
            model->indices.data(),
            GL_STATIC_DRAW);
    }
    // TODO: Move above into model

    // Use the program object
    GLuint programId = reinterpret_cast<GLuint>(shader.shader->opaque);
    glUseProgram(programId);

    // Load VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, 0, sizeof(decltype(model->vertices)::value_type), 0);
    glEnableVertexAttribArray(0);

    // Load IBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    // Bind MVP
    auto mvpLoc = glGetUniformLocation(programId, "uMVP");
    if (mvpLoc != -1)
    {
        const glm::mat4 model = glm::translate(rot, pos);
        const glm::mat4 mvp = g_camera.projection * g_camera.view * model;
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
    }

    glDrawElements(GL_TRIANGLES, model->indices.size(), GL_UNSIGNED_SHORT, 0);
}
