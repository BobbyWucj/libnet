#include "core/TcpServer.h"
#include "core/TcpConnection.h"
#include <map>

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
    Timer::sptr timer_;
    ConnectionList connections_;
};