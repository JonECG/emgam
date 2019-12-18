#pragma once

#include "context.h"

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

#include <map>
#include <string>

namespace emsg
{
    struct Shader
    {
        struct Property
        {
            glm::mat4 value;
            enum class Type { Float, Int, Vector, Matrix } type;
            const void* opaque;
        };

        using Properties = std::map<std::string, Property>;
        struct Instance
        {
            const Shader* shader;
            Properties properties;
        };

        Properties defaultProperties;
        const void* opaque;

        static Shader* Get(const std::string& name);
        static Shader* Make(const std::string& name, const std::string& vert, const std::string& frag, Shader::Properties&& props = {});
        Shader::Instance Instantiate() const;
    };

    struct Renderable
    {
        Shader::Instance shader;
        glm::vec3 pos;
        glm::quat rot;

        static void Clear(const Context& ctx);
        void Draw() const;
    };
}
