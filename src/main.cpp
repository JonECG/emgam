#include "context.h"
#include "loop.h"

using namespace emsg;

Context s_ctx;

void frame()
{
    s_ctx.FrameBegin();
    Tick(s_ctx);
}

#if EMSCRIPTEN
#include <emscripten.h>
int main(int argc, char *argv[])
#else
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#endif
{
    s_ctx = Context::Create();
    Init(s_ctx);
#ifdef __EMSCRIPTEN__
    // void emscripten_set_main_loop(em_callback_func func, int fps, int simulate_infinite_loop);
    emscripten_set_main_loop(frame, 60, 1);
#else
    while (1) 
    {
        frame();
    }
   #endif
    return 0;
}
