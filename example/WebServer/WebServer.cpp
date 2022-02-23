#include "WebServer.h"
#include "HttpResponse.h"
#include "HttpParser.h"

#include "libnet/base/Logger.h"
#include "libnet/Buffer.h"
#include "libnet/TcpConnection.h"

#include <any>
#include <functional>

using namespace webserver;
using namespace libnet;

namespace webserver {
namespace detail {

void defaultHttpCallback(const HttpRequest& , HttpResponse* response) {
    response->setStatusCode(HttpResponse::k404NotFound);
    response->setStatusMessage("Not Found");
    response->closeConnection();
}

} // namespace detail
} // namespace webserver

WebServer::WebServer(EventLoop* loop, 
                     const InetAddress& listenAddr)
    : server_(loop, listenAddr),
      httpCallback_(detail::defaultHttpCallback)
{
    server_.setConnectionCallback(
        std::bind(&WebServer::onConnection, this, _1)
    );

    server_.setMessageCallback(
        std::bind(&WebServer::onMessage, this, _1, _2)
    );
}

void WebServer::start() {
    LOG_WARN << "WebServer starts listening on " << server_.ipPort();
    server_.start();
}

void WebServer::onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
        conn->setContext(std::make_any<HttpParser>());
    }
}

void WebServer::onMessage(const TcpConnectionPtr& conn, Buffer& buffer) {
    HttpParser* parser = std::any_cast<HttpParser>(conn->getMutableContext());

    if (!parser->parseRequest(buffer)) {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }

    if (parser->gotAll()) {
        onRequest(conn, parser->request());
        parser->reset();
    }
}

void WebServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& request) {
    const string& connection = request.getHeader("Connection");
    bool close = (connection == "close") || 
                 (request.version() == HttpRequest::kHttp10 && connection != "Keep-Alive");
    
    HttpResponse response(close);
    httpCallback_(request, &response);

    Buffer buffer;
    response.appendToBuffer(buffer);

    conn->send(buffer);
    if (response.closeConnection()) {
        conn->shutdown();
    }
}
