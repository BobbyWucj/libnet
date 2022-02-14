#include <cstddef>
#include <functional>
#include <map>

#include <libnet/EventLoop.h>
#include <libnet/TcpServer.h>
#include <libnet/TcpConnection.h>
#include <libnet/Logger.h>

using namespace std::placeholders;
using namespace libnet;

class EchoServer
{
public:
    EchoServer(EventLoop* loop, const InetAddress& addr, size_t numThread = 1, Nanoseconds timeout = 5s);
    ~EchoServer();

    void start();

    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer& buffer);
    void onHighWaterMark(const TcpConnectionPtr& conn, size_t mark);
    void onWriteComplete(const TcpConnectionPtr& conn);

private:
    void expireAfter(const TcpConnectionPtr& conn, const Nanoseconds interval);

    void onTimeout();

private:
    using ConnectionList = std::map<TcpConnectionPtr, Timestamp>;

    EventLoop* loop_;
    TcpServer server_;
    const size_t numThread_;
    const Nanoseconds timeout_;
    Timer* timer_;
    ConnectionList connections_;
};

inline EchoServer::EchoServer(EventLoop* loop, const InetAddress& addr, size_t numThread, Nanoseconds timeout)
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

inline EchoServer::~EchoServer()
{
    loop_->cancelTimer(timer_);
}

inline void EchoServer::start() {
    server_.setNumThreads(numThread_);
    server_.start();
}

inline void EchoServer::onConnection(const TcpConnectionPtr& conn) {
    LOG_INFO("connection %s is [%s]",
             conn->name().c_str(),
             conn->connected() ? "up":"down");
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

inline void EchoServer::onMessage(const TcpConnectionPtr& conn, Buffer& buffer) {
    LOG_TRACE("connection %s recv %lu bytes",
             conn->name().c_str(),
             buffer.readableBytes());

    conn->send(buffer);
    expireAfter(conn, timeout_);
}

inline void EchoServer::onHighWaterMark(const TcpConnectionPtr& conn, size_t mark) {
    LOG_INFO("high water mark %lu bytes, stop read", mark);
    conn->stopRead();
    expireAfter(conn, 2 * timeout_);
}

inline void EchoServer::onWriteComplete(const TcpConnectionPtr& conn) {
    if (!conn->isReading()) {
        LOG_INFO("write complete, start read");
        conn->startRead();
        expireAfter(conn, timeout_);
    }
}

inline void EchoServer::expireAfter(const TcpConnectionPtr& conn, const Nanoseconds interval) {
    connections_[conn] = clock::nowAfter(interval);
}

inline void EchoServer::onTimeout() {
    for (auto it = connections_.begin(); it != connections_.end(); ) {
        if (it->second <= clock::now()) {
            LOG_INFO("connection %s timeout force close", it->first->name().c_str());
            it->first->forceClose();
            it = connections_.erase(it);
        } else {
            ++it;
        }
    }
}

int main()
{
    setLogLevel(LOG_LEVEL_TRACE);
    EventLoop loop;
    InetAddress addr(9877);

    EchoServer server(&loop, addr, 1, 5s);
    server.start();

    loop.runAfter(100s, [&](){
        int countdown = 5;
        LOG_INFO("server quit after %d second...", countdown);
        loop.runEvery(1s, [&, countdown]() mutable {
            LOG_INFO("server quit after %d second...", --countdown);
            if (countdown == 0)
                loop.quit();
        });
    });

    loop.loop();
}

