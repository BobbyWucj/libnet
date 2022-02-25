#ifndef LIBNET_TCPSERVERSINGLE_H
#define LIBNET_TCPSERVERSINGLE_H

#include "Acceptor.h"
#include "Callbacks.h"
#include "libnet/Timestamp.h"
#include "libnet/base/noncopyable.h"

#include <memory>
#include <unordered_set>

namespace libnet 
{

class EventLoop;

class TcpServerSingle : noncopyable 
{
public:
    TcpServerSingle(EventLoop* loop, const InetAddress& local, const Nanoseconds heartbeat);

    void start();

    void setConnectionCallback(const ConnectionCallback &connectionCallback) 
    { connectionCallback_ = connectionCallback; }

    void setMessageCallback(const MessageCallback &messageCallback) 
    { messageCallback_ = messageCallback; }

    void setWriteCompleteCallback(const WriteCompleteCallback &writeCompleteCallback) 
    { writeCompleteCallback_ = writeCompleteCallback; }

private:
    void newConnection(int connfd, const InetAddress& local, const InetAddress& peer);
    void closeConnection(const TcpConnectionPtr& conn);

    using ConnectionSet = std::unordered_set<TcpConnectionPtr>;

    EventLoop*                  loop_;
    std::unique_ptr<Acceptor>   acceptor_;
    ConnectionSet               connections_;
    ConnectionCallback          connectionCallback_;
    MessageCallback             messageCallback_;
    WriteCompleteCallback       writeCompleteCallback_;
    Nanoseconds                 heartbeat_;
};

} // namespace libnet

#endif // LIBNET_TCPSERVERSINGLE_H
