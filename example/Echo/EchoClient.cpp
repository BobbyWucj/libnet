#include <iostream>
#include <thread>

#include "EchoClient.h"
#include "core/EventLoop.h"
#include "core/InetAddress.h"
#include "logger/Logger.h"

using namespace libnet;

EchoClient::EchoClient(EventLoop* loop, const InetAddress& peer)
    : loop_(loop), client_(loop, peer) {
    client_.setConnectionCallback(
        std::bind(&EchoClient::onConnection, this, _1));
}

void EchoClient::start() {
    client_.start();
}

void EchoClient::onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
        conn->setMessageCallback(
            std::bind(&EchoClient::onMessage, this, _1, _2));
        conn_ = conn;
        auto thread = std::thread([this]() { this->getLineAndSend(); });
        thread.detach();
    }
    else {
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
}

int main() {
    Logger::setLogLevel(Logger::TRACE);
    EventLoop loop;
    InetAddress peer("172.31.179.98", 9877);
    EchoClient client(&loop, peer);
    client.start();
    loop.loop();
    return 0;
}