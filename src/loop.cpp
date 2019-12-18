#include "camera.h"
#include "loop.h"
#include "renderable.h"

using namespace emsg;

namespace
{
    Renderable r;
    Model m;
    float s_step = 0;
}

void emsg::Init(const Context& ctx)
{
    Shader* s = Shader::Make("Passthru",
        "attribute vec4 vPosition;    \n"
        "uniform mat4 uMVP;           \n"
        "varying vec4 vColor;         \n"
        "void main()                  \n"
        "{                            \n"
        "   vColor = vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);   \n"
        "   gl_Position = uMVP * vPosition;  \n"
        "}                            \n",

        "precision mediump float; \n"
        "varying vec4 vColor;     \n"
        "void main()              \n"
        "{                        \n"
        "  gl_FragColor = vColor; \n"
        "}                        \n",
        {});
    m = Model::CreateBox(1);

    r.shader = s->Instantiate();
    r.model = &m;
}

void emsg::Tick(const Context& ctx)
{
    s_step += 0.01f;
    glm::vec3 v = { glm::cos(s_step) * 5, 2, glm::sin(s_step) * 5 };
    g_camera.view = glm::lookAt(v, { 0, 0, 0 }, { 0, 1, 0 });
    g_camera.projection = glm::perspective(1.0f, ctx.width / float(ctx.height), 0.1f, 10.0f);

    Renderable::Clear(ctx);
    r.Draw();
}
