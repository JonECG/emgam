#pragma once

#include <stddef.h>

namespace emsg
{
    struct WebSocket
    {
        struct Server
        {
            bool Listen(const char* ip, unsigned short port);
            bool Accept(WebSocket& socket);
            bool Close();

        private:
            void* opaque;
        };

        static bool Connect(WebSocket& socket, const char* ip, unsigned short port);
        bool Disconnect();

        bool Send(const void* data, size_t size);
        bool Receive(void* buffer, size_t bufferSize, size_t& receivedBytes);

    private:
        void* opaque;
    };
}
