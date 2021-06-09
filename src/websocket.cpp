#include "websocket.h"

using namespace emsg;

#if EMSCRIPTEN
#include <emscripten/emscripten.h>
#include <emscripten/websocket.h>

#else
#define _WEBSOCKETPP_CPP11_STL_
#define ASIO_STANDALONE 
#include <deque>
#include <mutex>
#include <string>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/server.hpp>

using wClient = websocketpp::client<websocketpp::config::asio_client>;
using wServer = websocketpp::server<websocketpp::config::asio>;

namespace
{
    struct InternalServer
    {
        void OnMsg(websocketpp::connection_hdl, wClient::message_ptr msg)
        {
            std::lock_guard<std::mutex> lock(mtx);
            msgs.push_back(msg);
        }

        void Echo(websocketpp::connection_hdl hdl, wServer::message_ptr msg) {
            server.send(hdl, msg->get_payload(), msg->get_opcode());
        }

        wServer server;
        wServer::connection_ptr con;
        std::mutex mtx;
        std::deque<wClient::message_ptr> msgs;
    };

    struct InternalClient
    {
        void OnMsg(websocketpp::connection_hdl, wClient::message_ptr msg)
        {
            std::lock_guard<std::mutex> lock(mtx);
            msgs.push_back(msg);
        }

        wClient client;
        wClient::connection_ptr con;
        std::mutex mtx;
        std::deque<wClient::message_ptr> msgs;
    };
}


bool WebSocket::Server::Listen(const char* ip, unsigned short port)
{
    InternalServer* ise = new InternalServer;
    opaque = ise;

    ise->server.set_message_handler(std::bind(
        &InternalServer::Echo, ise,
        std::placeholders::_1, std::placeholders::_2
    ));

    ise->server.listen(port);
    ise->server.start_accept();

    return true;
}

bool WebSocket::Server::Accept(WebSocket& socket)
{
    return false;
}

bool WebSocket::Server::Close()
{
    return false;
}

bool WebSocket::Connect(WebSocket& socket, const char* ip, unsigned short port)
{
    InternalClient* icl = new InternalClient;
    socket.opaque = icl;

    websocketpp::lib::error_code ec;
    icl->con =
        icl->client.get_connection(std::string(ip) + ":" + std::to_string(port), ec);

    icl->con->set_message_handler(websocketpp::lib::bind(
        &InternalClient::OnMsg,
        icl,
        websocketpp::lib::placeholders::_1,
        websocketpp::lib::placeholders::_2
    ));

    icl->client.connect(icl->con);
    return !ec;
}

bool WebSocket::Disconnect()
{
    InternalClient* icl = reinterpret_cast<InternalClient*>(opaque);
    icl->client.close(icl->con->get_handle(), websocketpp::close::status::going_away, "Disconnected like a goodboi");
    return true;
}

bool WebSocket::Send(const void* data, size_t size)
{
    InternalClient* icl = reinterpret_cast<InternalClient*>(opaque);
    websocketpp::lib::error_code ec;
    icl->client.send(icl->con->get_handle(), data, size, websocketpp::frame::opcode::binary, ec);
    return !ec;
}

bool WebSocket::Receive(void* buffer, size_t bufferSize, size_t& receivedBytes)
{
    InternalClient* icl = reinterpret_cast<InternalClient*>(opaque);
    std::lock_guard<std::mutex> lock(icl->mtx);

    if (icl->msgs.empty())
    {
        return false;
    }

    const auto& msg = icl->msgs.front()->get_payload();
    receivedBytes = msg.size();
    if (bufferSize < msg.size())
    {
        return false;
    }

    memcpy(buffer, msg.data(), receivedBytes);
    icl->msgs.pop_front();
    return true;
}
#endif
