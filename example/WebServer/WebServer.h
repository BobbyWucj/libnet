#ifndef EXAMPLE_WEBSERVER_WEBSERVER_H
#define EXAMPLE_WEBSERVER_WEBSERVER_H

#include "HttpResponse.h"
#include "core/Buffer.h"
#include "core/Callbacks.h"
#include "core/EventLoop.h"
#include "core/InetAddress.h"
#include "core/TcpServer.h"
#include "utils/noncopyable.h"
#include <cstddef>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace libnet {

class Buffer;
class EventLoop;
class InetAddress;
class Timer;

}  // namespace libnet

namespace webserver {

class HttpRequest;

using libnet::Buffer;
using libnet::EventLoop;
using libnet::InetAddress;
using libnet::TcpConnectionPtr;

class WebServer : libnet::noncopyable
{
public:
    using HttpCallback = std::function<void(const HttpRequest&, HttpResponse*)>;

    WebServer(EventLoop* loop,
              const InetAddress& listenAddr,
              const std::string& root = "./example/WebServer/root");

    EventLoop* loop() const { return server_.getLoop(); }

    void setHttpCallback(const HttpCallback& httpCallback) {
        httpCallback_ = httpCallback;
    }

    void setNumThreads(size_t numThreads) { server_.setNumThreads(numThreads); }

    void start();

    std::string root() const { return root_; }
    void setRoot(const std::string& root) { root_ = root; }

    void disableReusePort() { server_.disableReusePort(); }

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer& buffer);
    void onRequest(const TcpConnectionPtr& conn, const HttpRequest& request);
    void onHttp(const HttpRequest& request, HttpResponse* response);
    void onError(HttpResponse* response,
                 const HttpResponse::HttpStatusCode statusCode);

    libnet::TcpServer server_;
    HttpCallback httpCallback_;
    std::string root_;  // WebServer root directory
};

}  // namespace webserver

#endif  // EXAMPLE_WEBSERVER_WEBSERVER_H
