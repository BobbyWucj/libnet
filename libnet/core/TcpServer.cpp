/*
 * TcpServer.cpp
 * Created on 2022/3/9
 * Copyright (c) 2022 BobbyWucj
 *
 * TCP Server
 */

#include "core/TcpServer.h"
#include "core/EventLoop.h"
#include "core/EventLoopThreadPool.h"
#include "core/TcpConnection.h"
#include "core/TcpMainReactor.h"
#include "core/TcpSubReactor.h"
#include "logger/Logger.h"
#include <memory>
#include <utility>

using namespace libnet;

TcpServer::TcpServer(EventLoop*         loop,
                     const InetAddress& local,
                     bool               reusePort,
                     const Nanoseconds  heartbeat)
    : baseLoop_(loop),
      numThreads_(1),
      started_(false),
      local_(local),
      ipPort_(local.toIpPort()),
      heartbeat_(heartbeat),
      reusePort_(reusePort),
      threadInitCallback_(defaultThreadInitCallback),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback) {
    LOG_TRACE << "Creating TcpServer() " << local.toIpPort();
}

TcpServer::~TcpServer() {
    LOG_TRACE << "~TcpServer() " << local_.toIpPort();
}

void TcpServer::setNumThreads(size_t numThreads) {
    baseLoop_->assertInLoopThread();
    assert(numThreads > 0);
    assert(!started_);
    numThreads_ = numThreads;
}

void TcpServer::start() {
    if (started_.exchange(true)) {
        return;
    }
    baseLoop_->runInLoop([this]() { this->startInLoop(); });
}

void TcpServer::startInLoop() {
    LOG_INFO << "TcpServer::start() " << local_.toIpPort() << " with "
             << numThreads_ << " eventLoop thread(s)";

    if (reusePort_) {
        auto reactor =
            std::make_unique<TcpSubReactor>(baseLoop_, local_, heartbeat_);
        reactor->setNumThreads(numThreads_);
        reactor_ = std::move(reactor);
    }
    else {
        auto reactor =
            std::make_unique<TcpMainReactor>(baseLoop_, local_, heartbeat_);
        reactor->setNumThreads(numThreads_);
        reactor_ = std::move(reactor);
    }

    reactor_->setConnectionCallback(connectionCallback_);
    reactor_->setMessageCallback(messageCallback_);
    reactor_->setWriteCompleteCallback(writeCompleteCallback_);

    // main thread
    threadInitCallback_(0);

    reactor_->start();
}
