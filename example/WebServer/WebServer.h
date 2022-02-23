#ifndef EXAMPLE_WEBSERVER_WEBSERVER_H
#define EXAMPLE_WEBSERVER_WEBSERVER_H

#include "libnet/Buffer.h"
#include "libnet/Callbacks.h"
#include "libnet/EventLoop.h"
#include "libnet/InetAddress.h"
#include "libnet/base/noncopyable.h"
#include "libnet/TcpServer.h"
#include <cstddef>
#include <functional>
#include <string>

namespace libnet 
{

class Buffer;
class EventLoop;
class InetAddress;

}

namespace webserver
{

class HttpRequest;
class HttpResponse;

using libnet::TcpConnectionPtr;
using libnet::Buffer;
using libnet::EventLoop;
using libnet::InetAddress;

class WebServer : libnet::noncopyable
{
public:
    using HttpCallback = std::function<void(const HttpRequest&, HttpResponse*)>;

    WebServer(EventLoop* loop, const InetAddress& listenAddr);

    EventLoop* getLoop() const {
        return server_.getLoop();
    }

    void setHttpCallback(const HttpCallback &httpCallback) 
    { httpCallback_ = httpCallback; }

    void setNumThreads(size_t numThreads) {
        server_.setNumThreads(numThreads);
    }

    void start();

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer& buffer);
    void onRequest(const TcpConnectionPtr& conn, const HttpRequest& request);

    libnet::TcpServer server_;
    HttpCallback httpCallback_;
};

}

#endif // EXAMPLE_WEBSERVER_WEBSERVER_H
