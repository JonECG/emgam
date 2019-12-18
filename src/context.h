#pragma once

namespace emsg
{
    struct Context
    {
        /// Put your user data here...
        void* userData;

        /// Window width
        int width;

        /// Window height
        int height;


        /// Callbacks
        void (*drawFunc) (struct Context *);
        void (*keyFunc) (struct Context *, unsigned char, int, int);
        void (*updateFunc) (struct Context *, float deltaTime);

        void FrameBegin();
        static Context Create();
    };
}
