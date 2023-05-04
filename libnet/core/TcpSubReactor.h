/*
 * TcpServerSingle.h
 * Created on 2023/3/9
 * Copyright (c) 2023 BobbyWucj
 *
 * Sub Reactor
 */

#ifndef LIBNET_TCPSERVERSINGLE_H
#define LIBNET_TCPSERVERSINGLE_H

#include "core/TcpReactor.h"

#include <condition_variable>
#include <memory>
#include <mutex>

namespace libnet {

class EventLoop;

class TcpSubReactor : public TcpReactor
{
public:
    using ptr = std::unique_ptr<TcpSubReactor>;
    using ThreadPtr = std::unique_ptr<std::thread>;
    using ThreadPtrList = std::vector<ThreadPtr>;
    using EventLoopList = std::vector<EventLoop*>;

    TcpSubReactor(EventLoop* loop,
                  const InetAddress& local,
                  const Nanoseconds heartbeat);
    ~TcpSubReactor() = default;

    void setNumThreads(size_t numThreads) override;
    void start() override;

private:
    void newConnection(int connfd,
                       const InetAddress& local,
                       const InetAddress& peer) override;
    void closeConnection(const TcpConnectionPtr& conn) override;

    void runInThread(const size_t index);

    ThreadPtrList threads_;
    EventLoopList eventLoops_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

}  // namespace libnet

#endif  // LIBNET_TCPSERVERSINGLE_H
