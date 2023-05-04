/*
 * TcpServerReactor.h
 * Created on 2023/3/1
 * Copyright (c) 2023 BobbyWucj
 *
 * Main Reactor
 */

#ifndef LIBNET_TCPSERVERREACTOR_H
#define LIBNET_TCPSERVERREACTOR_H

#include "core/EventLoopThreadPool.h"
#include "core/TcpReactor.h"

namespace libnet {

class TcpMainReactor : public TcpReactor
{
public:
    using ptr = std::unique_ptr<TcpMainReactor>;

    TcpMainReactor(EventLoop* loop,
                   const InetAddress& local,
                   const Nanoseconds heartbeat);
    ~TcpMainReactor() = default;

    // Main Reactor will create numThreads-1 threads and loop
    // only Main Reactor own an acceptor
    // when new-connection came,
    // Main Reactor will distribute TcpConnection to EventLoopThreadPool with
    // Round-Robin
    void setNumThreads(size_t numThreads) override;
    void start() override;

    size_t numThreads() const { return threadPool_->numThreads(); }

private:
    void newConnection(int connfd,
                       const InetAddress& local,
                       const InetAddress& peer) override;
    void closeConnection(const TcpConnectionPtr& conn) override;
    void closeConnectionInLoop(const TcpConnectionPtr& connPtr);

    EventLoopThreadPool::ptr threadPool_;
};

}  // namespace libnet

#endif  // LIBNET_TCPSERVERREACTOR_H