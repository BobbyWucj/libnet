#ifndef LIBNET_TCPSERVERSINGLE_H
#define LIBNET_TCPSERVERSINGLE_H

#include "Acceptor.h"
#include "Callbacks.h"
#include "libnet/Timestamp.h"
#include "libnet/base/noncopyable.h"
#include "libnet/EventLoopThreadPool.h"

#include <cstddef>
#include <memory>
#include <unordered_set>

namespace libnet 
{

class EventLoop;

class TcpServerSingle : noncopyable 
{
public:
    using ConnectionSet = std::unordered_set<TcpConnectionPtr>;

    TcpServerSingle(EventLoop* loop, const InetAddress& local, const Nanoseconds heartbeat);

    void start();

    void setConnectionCallback(const ConnectionCallback &connectionCallback) 
    { connectionCallback_ = connectionCallback; }

    void setMessageCallback(const MessageCallback &messageCallback) 
    { messageCallback_ = messageCallback; }

    void setWriteCompleteCallback(const WriteCompleteCallback &writeCompleteCallback) 
    { writeCompleteCallback_ = writeCompleteCallback; }

    void closeConnection(const TcpConnectionPtr& conn);

    ConnectionSet connections() const { return connections_; }

    void disableReusePort(size_t numThreads);

    size_t threadNum() const;

private:
    void newConnection(int connfd, const InetAddress& local, const InetAddress& peer);

    EventLoop*                  loop_;
    std::unique_ptr<Acceptor>   acceptor_;
    ConnectionSet               connections_;
    ConnectionCallback          connectionCallback_;
    MessageCallback             messageCallback_;
    WriteCompleteCallback       writeCompleteCallback_;
    Nanoseconds                 heartbeat_;

    bool reusePort_;
    std::unique_ptr<EventLoopThreadPool> threadPool_;
};

} // namespace libnet

#endif // LIBNET_TCPSERVERSINGLE_H
