#include <cstddef>
#include <functional>

#include "core/Callbacks.h"
#include "core/EventLoop.h"
#include "logger/Logger.h"
#include "EchoServer.h"

using namespace std::placeholders;

EchoServer::EchoServer(EventLoop* loop, const InetAddress& addr, size_t numThread, Nanoseconds timeout)
    : loop_(loop),
      server_(loop, addr),
      numThread_(numThread),
      timeout_(timeout),
      timer_(loop_->runEvery(timeout_, [this](){ this->onTimeout(); }))
{
    server_.setConnectionCallback(std::bind(
        &EchoServer::onConnection, this, _1
    ));

    server_.setMessageCallback(std::bind(
        &EchoServer::onMessage, this, _1, _2
    ));

    server_.setWriteCompleteCallback(std::bind(
        &EchoServer::onWriteComplete, this, _1
    ));
}

EchoServer::~EchoServer()
{
    loop_->cancelTimer(timer_);
}

void EchoServer::start() {
    server_.setNumThreads(numThread_);
    server_.start();
}

void EchoServer::onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
        conn->setHighWaterMarkCallback(
            std::bind(&EchoServer::onHighWaterMark, this, _1, _2), 
            1024
        );
        expireAfter(conn, timeout_);
    } else {
        connections_.erase(conn);
    }
}

void EchoServer::onMessage(const TcpConnectionPtr& conn, Buffer& buffer) {
    conn->send(buffer);
    expireAfter(conn, timeout_);
}

void EchoServer::onHighWaterMark(const TcpConnectionPtr& conn, size_t mark) {
    LOG_INFO << "Reached High water mark " << mark << " bytes, stop read";
    conn->stopRead();
    expireAfter(conn, 2 * timeout_);
}

void EchoServer::onWriteComplete(const TcpConnectionPtr& conn) {
    if (!conn->isReading()) {
        LOG_INFO << "Write complete, start read";
        conn->startRead();
        expireAfter(conn, timeout_);
    }
}

void EchoServer::expireAfter(const TcpConnectionPtr& conn, const Nanoseconds interval) {
    connections_[conn] = clock::nowAfter(interval);
}

void EchoServer::onTimeout() {
    for (auto it = connections_.begin(); it != connections_.end(); ) {
        if (it->second <= clock::now()) {
            LOG_INFO << "connection " << it->first->name() << "  timeout force close";
            it->first->forceClose();
            it = connections_.erase(it);
        } else {
            ++it;
        }
    }
}

int main()
{
    Logger::setLogLevel(Logger::ERROR);
    EventLoop loop;
    InetAddress addr(9877);

    EchoServer server(&loop, addr, 1, 5s);
    server.start();

    loop.runAfter(100s, [&](){
        int countdown = 5;
        LOG_INFO << "server quit after "<< countdown << " second...";
        loop.runEvery(1s, [&, countdown]() mutable {
            --countdown;
            LOG_INFO << "server quit after "<< countdown << " second...";
            if (countdown == 0)
                loop.quit();
        });
    });

    loop.loop();
}

