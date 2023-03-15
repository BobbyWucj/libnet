/*
 * TcpReactor.h
 * Created on 2023/3/9
 * Copyright (c) 2023 BobbyWucj
 *
 * Reactor 基类
 */

#ifndef TCPREACTOR_H
#define TCPREACTOR_H

#include "Acceptor.h"
#include "Callbacks.h"
#include "libnet/Timestamp.h"
#include "libnet/base/noncopyable.h"
#include "libnet/EventLoopThreadPool.h"
#include "libnet/TimerQueue.h"

#include <cstddef>
#include <memory>
#include <unordered_set>
#include <atomic>

namespace libnet
{

class TcpReactor : noncopyable
{
public:
    using ptr = std::unique_ptr<TcpReactor>;
    using ConnectionSet = std::unordered_set<TcpConnectionPtr>;

    TcpReactor(EventLoop* loop, const InetAddress& local, const Nanoseconds heartbeat);
    virtual ~TcpReactor();

    virtual void start() = 0;

    void setConnectionCallback(const ConnectionCallback &connectionCallback) 
    { connectionCallback_ = connectionCallback; }

    void setMessageCallback(const MessageCallback &messageCallback) 
    { messageCallback_ = messageCallback; }

    void setWriteCompleteCallback(const WriteCompleteCallback &writeCompleteCallback) 
    { writeCompleteCallback_ = writeCompleteCallback; }

    ConnectionSet connections() const { return connections_; }

protected:
    virtual void newConnection(int connfd, const InetAddress& local, const InetAddress& peer) = 0;
    virtual void closeConnection(const TcpConnectionPtr& conn) = 0;
    virtual void closeConnectionInLoop(const TcpConnectionPtr &connPtr) = 0;

    EventLoop*                  loop_;
    Acceptor::ptr               acceptor_;
    ConnectionSet               connections_;
    ConnectionCallback          connectionCallback_;
    MessageCallback             messageCallback_;
    WriteCompleteCallback       writeCompleteCallback_;
    Nanoseconds                 heartbeat_;
    std::atomic_bool            started_;
};
}

#endif // TCPREACTOR_H
