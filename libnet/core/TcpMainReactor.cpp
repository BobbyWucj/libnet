/*
 * TcpServerReactor.cpp
 * Created on 2023/3/3
 * Copyright (c) 2023 BobbyWucj
 *
 * Main Reactor
 */

#include "core/TcpMainReactor.h"
#include "core/Callbacks.h"
#include "core/EventLoop.h"
#include "core/TcpConnection.h"
#include <functional>
#include <memory>

using namespace libnet;

TcpMainReactor::TcpMainReactor(EventLoop* loop,
                               const InetAddress& local,
                               const Nanoseconds heartbeat)
    : TcpReactor(loop, local, heartbeat), threadPool_() {
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpMainReactor::newConnection, this, _1, _2, _3));
}

void TcpMainReactor::setNumThreads(size_t numThreads) {
    threadPool_ = std::make_shared<EventLoopThreadPool>(loop_, numThreads);
    numThreads_ = numThreads;
}

void TcpMainReactor::start() {
    if (started_.exchange(true) == false) {
        threadPool_->start();
        acceptor_->listen();
    }
}

void TcpMainReactor::newConnection(int connfd,
                                   const InetAddress& local,
                                   const InetAddress& peer) {
    loop_->assertInLoopThread();
    auto ioLoop = threadPool_->getNextLoop();

    auto connPtr = std::make_shared<TcpConnection>(ioLoop, connfd, local, peer,
                                                   heartbeat_);
    connections_.insert(connPtr);

    connPtr->setMessageCallback(messageCallback_);
    connPtr->setWriteCompleteCallback(writeCompleteCallback_);
    connPtr->setCloseCallback(
        std::bind(&TcpMainReactor::closeConnection, this, _1));
    connPtr->setConnectionCallback(connectionCallback_);

    ioLoop->runInLoop(
        std::bind(&TcpConnection::connectionEstablished, connPtr));
}

/* Erase connection in main loop */
void TcpMainReactor::closeConnection(const TcpConnectionPtr& connPtr) {
    loop_->runInLoop(
        std::bind(&TcpMainReactor::closeConnectionInLoop, this, connPtr));
}

void TcpMainReactor::closeConnectionInLoop(const TcpConnectionPtr& connPtr) {
    loop_->assertInLoopThread();
    auto ret = connections_.erase(connPtr);
    assert(ret == 1);
    (void)ret;
    // connPtr->forceClose(); // FIXME
    auto ioLoop = connPtr->getLoop();
    ioLoop->queueInLoop(
        std::bind(&TcpConnection::connectionDestroyed, connPtr));
}
