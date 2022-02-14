#include <iostream>
#include <thread>

#include "libnet/EventLoop.h"
#include "libnet/TcpClient.h"
#include "libnet/TcpConnection.h"
#include "libnet/Logger.h"

using namespace libnet;

class EchoClient : noncopyable
{
public:
    EchoClient(EventLoop* loop, const InetAddress& peer);

    void start();

    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer& buffer);
    void getLineAndSend();

private:
    TcpConnectionPtr conn_;
    EventLoop* loop_;
    TcpClient client_;
};

inline EchoClient::EchoClient(EventLoop* loop, const InetAddress& peer)
    : loop_(loop),
      client_(loop, peer)
{
    client_.setConnectionCallback(std::bind (
        &EchoClient::onConnection, this, _1
    ));
}

inline void EchoClient::start() {
    client_.start();
}

inline void EchoClient::onConnection(const TcpConnectionPtr& conn) {
    LOG_INFO("connection %s is [%s]",
        conn->name().c_str(),
        conn->connected() ? "up" : "down");

    if (conn->connected()) {
        conn->setMessageCallback(std::bind(
            &EchoClient::onMessage, this, _1, _2
        ));
        conn_ = conn;
        auto thread = std::thread([this]() {
            this->getLineAndSend();
        });
        thread.detach();
    } else {
        loop_->quit();
    }
}

inline void EchoClient::onMessage(const TcpConnectionPtr& conn, Buffer& buffer) {
    std::cout << buffer.retrieveAllAsString() << std::endl;
}

inline void EchoClient::getLineAndSend() {
    std::string line;
    while (std::getline(std::cin, line)) {
        conn_->send(line);
    }
    conn_->shutdown();
}

int main()
{
    setLogLevel(LOG_LEVEL_WARN);
    EventLoop loop;
    InetAddress peer("192.168.xxx.xxx", 9877);
    EchoClient client(&loop, peer);
    client.start();
    loop.loop();
}