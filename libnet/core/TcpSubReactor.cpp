/*
 * TcpSubReactor.cpp
 * Created on 2023/3/9
 * Copyright (c) 2023 BobbyWucj
 *
 * Sub Reactor
 */

#include "core/TcpSubReactor.h"
#include "core/Callbacks.h"
#include "core/EventLoop.h"
#include "core/TcpConnection.h"
#include "core/Timestamp.h"
#include "logger/Logger.h"
#include <cassert>
#include <functional>
#include <memory>
#include <utility>

using namespace libnet;

TcpSubReactor::TcpSubReactor(EventLoop* loop,
                             const InetAddress& local,
                             const Nanoseconds heartbeat)
    : TcpReactor(loop, local, heartbeat) {
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpSubReactor::newConnection, this, _1, _2, _3));
}

void TcpSubReactor::setNumThreads(size_t numThreads) {
    eventLoops_.resize(numThreads);
    numThreads_ = numThreads;
}

void TcpSubReactor::start() {
    acceptor_->listen();

    // create numThreads-1 threads and loop
    // every threads(loop) own an acceptor
    // and every threads is listening on the same port
    // The kernel will load-balance new connections to each thread
    for (size_t i = 1; i < numThreads_; ++i) {
        ThreadPtr thread = std::make_unique<std::thread>(
            [this, i]() { this->runInThread(i); });
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while (eventLoops_[i] == nullptr)
                cond_.wait(lock);
        }
        threads_.emplace_back(std::move(thread));
    }
}

void TcpSubReactor::newConnection(int connfd,
                                  const InetAddress& local,
                                  const InetAddress& peer) {
    loop_->assertInLoopThread();

    auto connPtr =
        std::make_shared<TcpConnection>(loop_, connfd, local, peer, heartbeat_);
    connections_.insert(connPtr);

    connPtr->setConnectionCallback(connectionCallback_);
    connPtr->setMessageCallback(messageCallback_);
    connPtr->setWriteCompleteCallback(writeCompleteCallback_);
    connPtr->setCloseCallback(
        std::bind(&TcpSubReactor::closeConnection, this, _1));
    connPtr->connectionEstablished();
}

void TcpSubReactor::closeConnection(const TcpConnectionPtr& connPtr) {
    loop_->assertInLoopThread();
    auto ret = connections_.erase(connPtr);
    assert(ret == 1);
    (void)ret;
    connPtr->connectionDestroyed();
}

void TcpSubReactor::runInThread(const size_t index) {
    EventLoop loop;
    // stack variable
    TcpSubReactor reactor(&loop, local_, heartbeat_);

    reactor.setConnectionCallback(connectionCallback_);
    reactor.setMessageCallback(messageCallback_);
    reactor.setWriteCompleteCallback(writeCompleteCallback_);

    {
        std::lock_guard<std::mutex> guard(mutex_);
        eventLoops_[index] = &loop;
        cond_.notify_one();
    }

    // threadInitCallback_(index);
    reactor.start();
    loop.loop();

    // stop looping
    eventLoops_[index] = nullptr;
}
