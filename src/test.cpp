//
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com
//

// Hello_Triangle.c
//
//    This is a simple example that draws a single triangle with
//    a minimal vertex/fragment shader.  The purpose of this 
//    example is to demonstrate the basic concepts of 
//    OpenGL ES 2.0 rendering.
#include <stdlib.h>

#if EMSCRIPTEN
    #include <GLES2/gl2.h>
    #include <EGL/egl.h>
    #include <emscripten.h>
#else
    #include <GL/glew.h>
    #include <GLFW/glfw3.h>
#endif

typedef struct
{
   // Handle to a program object
   GLuint programObject;

} UserData;

///
// Create a shader object, load the shader source, and
// compile the shader.
//
GLuint LoadShader ( GLenum type, const GLbyte *shaderSrc )
{
   GLuint shader;
   GLint compiled;
   
   // Create the shader object
   shader = glCreateShader ( type );

   if ( shader == 0 )
    return 0;

   // Load the shader source
   glShaderSource ( shader, 1, reinterpret_cast<const char**>(&shaderSrc), NULL );
   
   // Compile the shader
   glCompileShader ( shader );

   // Check the compile status
   glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

   if ( !compiled ) 
   {
      GLint infoLen = 0;

      glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 )
      {
         char* infoLog = static_cast<char*>(malloc(sizeof(char) * infoLen));

         glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
         //esLogMessage ( "Error compiling shader:\n%s\n", infoLog );            
         
         free ( infoLog );
      }

      glDeleteShader ( shader );
      return 0;
   }

   return shader;

}
#define ESUTIL_API
#define ESCALLBACK


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


///
// Types
//

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

typedef struct
{
    GLfloat   m[4][4];
} ESMatrix;

typedef struct _escontext
{
    /// Put your user data here...
    void*       userData;

    /// Window width
    GLint       width;

    /// Window height
    GLint       height;

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

    /// Callbacks
    void (ESCALLBACK *drawFunc) (struct _escontext *);
    void (ESCALLBACK *keyFunc) (struct _escontext *, unsigned char, int, int);
    void (ESCALLBACK *updateFunc) (struct _escontext *, float deltaTime);
} ESContext;

///
// Initialize the shader and program object
//
int Init ( ESContext *esContext )
{
   esContext->userData = malloc(sizeof(UserData));

   UserData *userData = reinterpret_cast<UserData*>(esContext->userData);
   GLbyte vShaderStr[] =  
      "attribute vec4 vPosition;    \n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = vPosition;  \n"
      "}                            \n";
   
   GLbyte fShaderStr[] =  
      "precision mediump float;\n"\
      "void main()                                  \n"
      "{                                            \n"
      "  gl_FragColor = vec4 ( 1.0, 0.0, 0.0, 1.0 );\n"
      "}                                            \n";

   GLuint vertexShader;
   GLuint fragmentShader;
   GLuint programObject;
   GLint linked;

   // Load the vertex/fragment shaders
   vertexShader = LoadShader ( GL_VERTEX_SHADER, vShaderStr );
   fragmentShader = LoadShader ( GL_FRAGMENT_SHADER, fShaderStr );

   // Create the program object
   programObject = glCreateProgram ( );
   
   if ( programObject == 0 )
      return 0;

   glAttachShader ( programObject, vertexShader );
   glAttachShader ( programObject, fragmentShader );

   // Bind vPosition to attribute 0   
   glBindAttribLocation ( programObject, 0, "vPosition" );

   // Link the program
   glLinkProgram ( programObject );

   // Check the link status
   glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );

   if ( !linked ) 
   {
      GLint infoLen = 0;

      glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 )
      {
         char* infoLog = static_cast<char*>(malloc (sizeof(char) * infoLen ));

         glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
         //esLogMessage ( "Error linking program:\n%s\n", infoLog );            
         
         free ( infoLog );
      }

      glDeleteProgram ( programObject );
      return GL_FALSE;
   }

   // Store the program object
   userData->programObject = programObject;

   glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
   return GL_TRUE;
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw ( ESContext *esContext )
{
   UserData *userData = reinterpret_cast<UserData*>(esContext->userData);
   GLfloat vVertices[] = {  0.0f,  0.5f, 0.0f, 
                           -0.5f, -0.5f, 0.0f,
                            0.5f, -0.5f, 0.0f };

   // No clientside arrays, so do this in a webgl-friendly manner
   GLuint vertexPosObject;
   glGenBuffers(1, &vertexPosObject);
   glBindBuffer(GL_ARRAY_BUFFER, vertexPosObject);
   glBufferData(GL_ARRAY_BUFFER, 9*4, vVertices, GL_STATIC_DRAW);
   
   // Set the viewport
   glViewport ( 0, 0, esContext->width, esContext->height );
   
   // Clear the color buffer
   glClear ( GL_COLOR_BUFFER_BIT );

   // Use the program object
   glUseProgram ( userData->programObject );

   // Load the vertex data
   glBindBuffer(GL_ARRAY_BUFFER, vertexPosObject);
   glVertexAttribPointer(0 /* ? */, 3, GL_FLOAT, 0, 0, 0);
   glEnableVertexAttribArray(0);

   glDrawArrays ( GL_TRIANGLES, 0, 3 );
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#if EMSCRIPTEN
#include <sys/time.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>

#include  <X11/Xlib.h>
#include  <X11/Xatom.h>
#include  <X11/Xutil.h>

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
EGLBoolean WinCreate(ESContext *esContext, const char *title)
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
        0, 0, esContext->width, esContext->height, 0,
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

    esContext->hWnd = (EGLNativeWindowType)win;
    return EGL_TRUE;
}

GLboolean ESUTIL_API esCreateWindow(ESContext *esContext, const char* title, GLint width, GLint height, GLuint flags)
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

    if (esContext == NULL)
    {
        return GL_FALSE;
    }

    esContext->width = width;
    esContext->height = height;

    if (!WinCreate(esContext, title))
    {
        return GL_FALSE;
    }


    if (!CreateEGLContext(esContext->hWnd,
        &esContext->eglDisplay,
        &esContext->eglContext,
        &esContext->eglSurface,
        attribList))
    {
        return GL_FALSE;
    }


    return GL_TRUE;
}

void ESUTIL_API esMainLoop(ESContext *esContext)
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

        if (esContext->updateFunc != NULL)
            esContext->updateFunc(esContext, deltatime);
        if (esContext->drawFunc != NULL)
            esContext->drawFunc(esContext);

        eglSwapBuffers(esContext->eglDisplay, esContext->eglSurface);

        totaltime += deltatime;
        frames++;
        if (totaltime >  2.0f)
        {
            printf("%4d frames rendered in %1.4f seconds -> FPS=%3.4f\n", frames, totaltime, frames / totaltime);
            totaltime -= 2.0f;
            frames = 0;
        }
    }
}
#else

GLboolean ESUTIL_API esCreateWindow(ESContext *esContext, const char* title, GLint width, GLint height, GLuint flags)
{
    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    esContext->window = glfwCreateWindow(width, height, title, 0, 0);

    if (!esContext->window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    //glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(esContext->window);

    if (glewInit() != GLEW_OK)
        exit(EXIT_FAILURE);
    //gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);
    
    return GL_TRUE;
}

void ESUTIL_API esMainLoop(ESContext *esContext)
{
    //struct timeval t1, t2;
    //struct timezone tz;
    float deltatime = 0.0f;
    float totaltime = 0.0f;
    unsigned int frames = 0;

    //gettimeofday(&t1, &tz);

    // Just one iteration! while(userInterrupt(esContext) == GL_FALSE)
    {
        //gettimeofday(&t2, &tz);
        //deltatime = (float)(t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6);
        //t1 = t2;

        glfwGetFramebufferSize(esContext->window, &esContext->width, &esContext->height);


        if (esContext->updateFunc != NULL)
            esContext->updateFunc(esContext, deltatime);
        if (esContext->drawFunc != NULL)
            esContext->drawFunc(esContext);

        glfwSwapBuffers(esContext->window);
        glfwPollEvents();

        totaltime += deltatime;
        frames++;
        if (totaltime >  2.0f)
        {
            printf("%4d frames rendered in %1.4f seconds -> FPS=%3.4f\n", frames, totaltime, frames / totaltime);
            totaltime -= 2.0f;
            frames = 0;
        }
    }
}
#endif

void ESUTIL_API esInitContext(ESContext *esContext)
{
    if (esContext != NULL)
    {
        memset(esContext, 0, sizeof(ESContext));
    }
}



void ESUTIL_API esRegisterDrawFunc(ESContext *esContext, void (ESCALLBACK *drawFunc) (ESContext*))
{
    esContext->drawFunc = drawFunc;
}

ESContext esContext;
   
void frame()
{
   esMainLoop ( &esContext );
}
#if EMSCRIPTEN
int main ( int argc, char *argv[] )
#else
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE , LPSTR, int)
#endif
{
   UserData  userData;

   esInitContext ( &esContext );
   esContext.userData = &userData;

   esCreateWindow ( &esContext, "Hello Triangle", 320, 240, ES_WINDOW_RGB );

   if ( !Init ( &esContext ) )
      return 0;

   esRegisterDrawFunc ( &esContext, Draw );

   #ifdef __EMSCRIPTEN__
       // void emscripten_set_main_loop(em_callback_func func, int fps, int simulate_infinite_loop);
       emscripten_set_main_loop(frame, 60, 1);
   #else
       while (1) {
        frame();
       }
   #endif
    return 0;
}
