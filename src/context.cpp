#include "context.h"

#if EMSCRIPTEN
    #include <emscripten.h>
    #include <GLES2/gl2.h>
    #include <EGL/egl.h>
#else
    #include <GL/glew.h>
    #include <GLFW/glfw3.h>
#endif

using namespace emsg;

/// esCreateWindow flag - RGB color buffer
#define ES_WINDOW_RGB           0
/// esCreateWindow flag - ALPHA color buffer
#define ES_WINDOW_ALPHA         1 
/// esCreateWindow flag - depth buffer
#define ES_WINDOW_DEPTH         2 
/// esCreateWindow flag - stencil buffer
#define ES_WINDOW_STENCIL       4
/// esCreateWindow flat - multi-sample buffer
#define ES_WINDOW_MULTISAMPLE   8

namespace
{
    struct Opaque
    {
#if EMSCRIPTEN
        /// Window handle
        EGLNativeWindowType  hWnd;

        /// EGL display
        EGLDisplay  eglDisplay;

        /// EGL context
        EGLContext  eglContext;

        /// EGL surface
        EGLSurface  eglSurface;
#else
        GLFWwindow* window;
#endif
    };

#if EMSCRIPTEN
}
#include <cstring>
#include <sys/time.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>

#include  <X11/Xlib.h>
#include  <X11/Xatom.h>
#include  <X11/Xutil.h>

#define FALSE 0
#define TRUE 1

namespace
{
    // X11 related local variables
    static Display *x_display = NULL;

    EGLBoolean CreateEGLContext(EGLNativeWindowType hWnd, EGLDisplay* eglDisplay,
        EGLContext* eglContext, EGLSurface* eglSurface,
        EGLint attribList[])
    {
        EGLint numConfigs;
        EGLint majorVersion;
        EGLint minorVersion;
        EGLDisplay display;
        EGLContext context;
        EGLSurface surface;
        EGLConfig config;
        EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };

        // Get Display
        long a = (long)x_display;
        //int display = int()
        display = eglGetDisplay((int)a);
        if (display == EGL_NO_DISPLAY)
        {
            return EGL_FALSE;
        }

        // Initialize EGL
        if (!eglInitialize(display, &majorVersion, &minorVersion))
        {
            return EGL_FALSE;
        }

        // Get configs
        if (!eglGetConfigs(display, NULL, 0, &numConfigs))
        {
            return EGL_FALSE;
        }

        // Choose config
        if (!eglChooseConfig(display, attribList, &config, 1, &numConfigs))
        {
            return EGL_FALSE;
        }

        // Create a surface
        surface = eglCreateWindowSurface(display, config, (EGLNativeWindowType)hWnd, NULL);
        if (surface == EGL_NO_SURFACE)
        {
            return EGL_FALSE;
        }

        // Create a GL context
        context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
        if (context == EGL_NO_CONTEXT)
        {
            return EGL_FALSE;
        }

        // Make the context current
        if (!eglMakeCurrent(display, surface, surface, context))
        {
            return EGL_FALSE;
        }

        *eglDisplay = display;
        *eglSurface = surface;
        *eglContext = context;
        return EGL_TRUE;
    }


    ///
    //  WinCreate()
    //
    //      This function initialized the native X11 display and window for EGL
    //
    EGLBoolean WinCreate(Opaque& opaque, const char *title, GLint width, GLint height)
    {
        Window root;
        XSetWindowAttributes swa;
        XSetWindowAttributes  xattr;
        Atom wm_state;
        XWMHints hints;
        XEvent xev;
        EGLConfig ecfg;
        EGLint num_config;
        Window win;

        /*
        * X11 native display initialization
        */

        x_display = XOpenDisplay(NULL);
        if (x_display == NULL)
        {
            return EGL_FALSE;
        }

        root = DefaultRootWindow(x_display);

        swa.event_mask = ExposureMask | PointerMotionMask | KeyPressMask;
        win = XCreateWindow(
            x_display, root,
            0, 0, width, height, 0,
            CopyFromParent, InputOutput,
            CopyFromParent, CWEventMask,
            &swa);

        xattr.override_redirect = FALSE;
        XChangeWindowAttributes(x_display, win, CWOverrideRedirect, &xattr);

        hints.input = TRUE;
        hints.flags = InputHint;
        XSetWMHints(x_display, win, &hints);

        // make the window visible on the screen
        XMapWindow(x_display, win);
        XStoreName(x_display, win, title);

        // get identifiers for the provided atom name strings
        wm_state = XInternAtom(x_display, "_NET_WM_STATE", FALSE);

        memset(&xev, 0, sizeof(xev));
        xev.type = ClientMessage;
        xev.xclient.window = win;
        xev.xclient.message_type = wm_state;
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = 1;
        xev.xclient.data.l[1] = FALSE;
        XSendEvent(
            x_display,
            DefaultRootWindow(x_display),
            FALSE,
            SubstructureNotifyMask,
            &xev);

        opaque.hWnd = (EGLNativeWindowType)win;
        return EGL_TRUE;
    }

    GLboolean esCreateWindow(Opaque& opaque, const char* title, GLint width, GLint height, GLuint flags)
    {
        EGLint attribList[] =
        {
            EGL_RED_SIZE,       5,
            EGL_GREEN_SIZE,     6,
            EGL_BLUE_SIZE,      5,
            EGL_ALPHA_SIZE,     (flags & ES_WINDOW_ALPHA) ? 8 : EGL_DONT_CARE,
            EGL_DEPTH_SIZE,     (flags & ES_WINDOW_DEPTH) ? 8 : EGL_DONT_CARE,
            EGL_STENCIL_SIZE,   (flags & ES_WINDOW_STENCIL) ? 8 : EGL_DONT_CARE,
            EGL_SAMPLE_BUFFERS, (flags & ES_WINDOW_MULTISAMPLE) ? 1 : 0,
            EGL_NONE
        };

        if (!WinCreate(opaque, title, width, height))
        {
            return GL_FALSE;
        }


        if (!CreateEGLContext(opaque.hWnd,
            &opaque.eglDisplay,
            &opaque.eglContext,
            &opaque.eglSurface,
            attribList))
        {
            return GL_FALSE;
        }

        return GL_TRUE;
    }

    void FrameBeginImpl(Context& ctx, Opaque& opq)
    {
        int width, height, fs;
        emscripten_get_canvas_size(&width, &height, &fs);
        ctx.width = int(width);
        ctx.height = int(height);
        eglSwapBuffers(opq.eglDisplay, opq.eglSurface);

    }
    void esMainLoop(Opaque& opaque)
    {
        struct timeval t1, t2;
        struct timezone tz;
        float deltatime;
        float totaltime = 0.0f;
        unsigned int frames = 0;

        gettimeofday(&t1, &tz);

        // Just one iteration! while(userInterrupt(esContext) == GL_FALSE)
        {
            gettimeofday(&t2, &tz);
            deltatime = (float)(t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6);
            t1 = t2;
            
            eglSwapBuffers(opaque.eglDisplay, opaque.eglSurface);

            totaltime += deltatime;
            frames++;
            if (totaltime >  2.0f)
            {
                //printf("%4d frames rendered in %1.4f seconds -> FPS=%3.4f\n", frames, totaltime, frames / totaltime);
                totaltime -= 2.0f;
                frames = 0;
            }
        }
    }
#else
    bool esCreateWindow(Opaque& esContext, const char* title, GLint width, GLint height, GLuint flags)
    {
        if (!glfwInit())
            return false;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

        esContext.window = glfwCreateWindow(width, height, title, 0, 0);

        if (!esContext.window)
        {
            glfwTerminate();
            return false;
        }

        //glfwSetKeyCallback(window, key_callback);
        glfwMakeContextCurrent(esContext.window);

        if (glewInit() != GLEW_OK)
            return false;
        glfwSwapInterval(1);

        return true;
    }

    void FrameBeginImpl(Context& ctx, Opaque& opq)
    {
        glfwGetFramebufferSize(opq.window, &ctx.width, &ctx.height);
        glfwSwapBuffers(opq.window);
        glfwPollEvents();
    }
#endif

}

void Context::FrameBegin()
{
    Opaque* opq = reinterpret_cast<Opaque*>(userData);
    FrameBeginImpl(*this, *opq);
}

Context Context::Create()
{
    Context result;
    Opaque* opq = new Opaque;
    result.userData = opq;
    esCreateWindow(*opq, "Hello Triangle", 320, 240, ES_WINDOW_RGB);
    return result;
}
