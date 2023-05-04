/*
 * TcpReactor.h
 * Created on 2023/3/9
 * Copyright (c) 2023 BobbyWucj
 *
 * Reactor 基类
 */

#ifndef TCPREACTOR_H
#define TCPREACTOR_H

#include "core/Acceptor.h"
#include "core/Callbacks.h"
#include "core/EventLoopThreadPool.h"
#include "core/TimerQueue.h"
#include "core/Timestamp.h"
#include "utils/noncopyable.h"

#include <atomic>
#include <cstddef>
#include <memory>
#include <unordered_set>

namespace libnet {

class TcpReactor : noncopyable
{
public:
    using ptr           = std::unique_ptr<TcpReactor>;
    using ConnectionSet = std::unordered_set<TcpConnectionPtr>;

    TcpReactor(EventLoop*         loop,
               const InetAddress& local,
               const Nanoseconds  heartbeat);
    virtual ~TcpReactor();

    virtual void setNumThreads(size_t numThreads) = 0;
    virtual void start()                          = 0;

    void setConnectionCallback(const ConnectionCallback& connectionCallback) {
        connectionCallback_ = connectionCallback;
    }

    void setMessageCallback(const MessageCallback& messageCallback) {
        messageCallback_ = messageCallback;
    }

    void setWriteCompleteCallback(
        const WriteCompleteCallback& writeCompleteCallback) {
        writeCompleteCallback_ = writeCompleteCallback;
    }

    ConnectionSet connections() const { return connections_; }

protected:
    virtual void newConnection(int                connfd,
                               const InetAddress& local,
                               const InetAddress& peer)        = 0;
    virtual void closeConnection(const TcpConnectionPtr& conn) = 0;

    EventLoop*            loop_;
    Acceptor::ptr         acceptor_;
    ConnectionSet         connections_;
    ConnectionCallback    connectionCallback_;
    MessageCallback       messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    Nanoseconds           heartbeat_;
    std::atomic_bool      started_;
    int                   numThreads_;
    InetAddress           local_;
};
}  // namespace libnet

#endif  // TCPREACTOR_H
