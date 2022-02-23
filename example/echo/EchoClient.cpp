#include <iostream>
#include <thread>

#include "libnet/EventLoop.h"
#include "libnet/InetAddress.h"
#include "libnet/base/Logger.h"
#include "EchoClient.h"

using namespace libnet;

EchoClient::EchoClient(EventLoop* loop, const InetAddress& peer)
    : loop_(loop),
      client_(loop, peer)
{
    client_.setConnectionCallback(std::bind (
        &EchoClient::onConnection, this, _1
    ));
}

void EchoClient::start() {
    client_.start();
}

void EchoClient::onConnection(const TcpConnectionPtr& conn) {
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

void EchoClient::onMessage(const TcpConnectionPtr& conn, Buffer& buffer) {
    std::cout << buffer.retrieveAllAsString() << std::endl;
}

void EchoClient::getLineAndSend() {
    std::string line;
    while (std::getline(std::cin, line)) {
        conn_->send(line);
    }
    conn_->shutdown();
}

int main()
{
    Logger::setLogLevel(Logger::ERROR);
    EventLoop loop;
    InetAddress peer("192.168.xxx.xxx", 9877);
    EchoClient client(&loop, peer);
    client.start();
    loop.loop();
    return 0;
}