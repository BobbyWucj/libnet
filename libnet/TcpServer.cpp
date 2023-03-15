/*
 * TcpServer.cpp
 * Created on 2022/3/9
 * Copyright (c) 2022 BobbyWucj
 *
 * TCP Server
 */

#include "TcpServer.h"
#include "libnet/EventLoopThreadPool.h"
#include "libnet/base/Logger.h"
#include "EventLoop.h"
#include "TcpMainReactor.h"
#include "TcpSubReactor.h"
#include "TcpConnection.h"
#include <memory>

using namespace libnet;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& local, bool reusePort, const Nanoseconds heartbeat)
    : baseLoop_(loop),
      numThreads_(1),
      started_(false),
      local_(local),
      ipPort_(local.toIpPort()),
      heartbeat_(heartbeat),
      reusePort_(reusePort),
      threadInitCallback_(defaultThreadInitCallback),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback)
{
    LOG_TRACE << "Creating TcpServer() " << local.toIpPort();
}

TcpServer::~TcpServer()
{
    for (auto& loop : eventLoops_) {
        if (loop != nullptr) {
            loop->quit();
        }
    }
    for (auto& thread : threads_) {
        thread->join();
    }
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
    baseLoop_->runInLoop([this]() {
                            this->startInLoop();
                        });
}

void TcpServer::startInLoop() {
    LOG_INFO << "TcpServer::start() " << local_.toIpPort() << " with " << numThreads_ << " eventLoop thread(s)";
    
    if (reusePort_) {
        reactor_ = std::make_unique<TcpSubReactor>(baseLoop_, local_, heartbeat_);
        eventLoops_.resize(numThreads_);
    } else {
        reactor_ = std::make_unique<TcpMainReactor>(baseLoop_, local_, heartbeat_);
        dynamic_cast<TcpMainReactor*>(reactor_.get())->setNumThreads(numThreads_);
    }

    reactor_->setConnectionCallback(connectionCallback_);
    reactor_->setMessageCallback(messageCallback_);
    reactor_->setWriteCompleteCallback(writeCompleteCallback_);

    // main thread
    threadInitCallback_(0);

    reactor_->start();

    // reusePort == true, will create numThreads-1 threads and loop
    // every threads(loop) own an acceptor
    // and every threads is listening on the same port
    // The kernel will load-balance new connections to each thread
    if (reusePort_) {
        for (size_t i = 1; i < numThreads_; ++i) {
            auto thread = new std::thread([this, i]() {
                                            this->runInThread(i);
                                        });
            {
                std::unique_lock<std::mutex> lock(mutex_);
                while (eventLoops_[i] == nullptr)
                    cond_.wait(lock);
            }
            threads_.emplace_back(thread);
        }
    }
}

// only called when reusePort_ == true
void TcpServer::runInThread(const size_t index) {
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

    threadInitCallback_(index);
    reactor.start();
    loop.loop();

    // stop looping
    eventLoops_[index] = nullptr;
}
