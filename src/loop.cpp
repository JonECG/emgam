#include "loop.h"
#include "renderable.h"

using namespace emsg;

namespace
{
    Renderable r;
}

void emsg::Init(const Context& ctx)
{
    Shader* s = Shader::Make("Passthru",
        "attribute vec4 vPosition;    \n"
        "void main()                  \n"
        "{                            \n"
        "   gl_Position = vPosition;  \n"
        "}                            \n",

        "precision mediump float;\n"\
        "void main()                                  \n"
        "{                                            \n"
        "  gl_FragColor = vec4 ( 1.0, 0.0, 0.0, 1.0 );\n"
        "}                                            \n",
        {});

    r.shader = s->Instantiate();
}

void emsg::Tick(const Context& ctx)
{
    Renderable::Clear(ctx);
    r.Draw();
}
