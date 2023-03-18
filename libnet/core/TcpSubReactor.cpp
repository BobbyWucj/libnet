/*
 * TcpSubReactor.cpp
 * Created on 2023/3/9
 * Copyright (c) 2023 BobbyWucj
 *
 * Sub Reactor
 */

#include "core/Callbacks.h"
#include "core/EventLoop.h"
#include "core/TcpSubReactor.h"
#include "core/TcpConnection.h"
#include "core/Timestamp.h"
#include "logger/Logger.h"
#include <cassert>
#include <functional>

using namespace libnet;

TcpSubReactor::TcpSubReactor(EventLoop* loop, const InetAddress& local, const Nanoseconds heartbeat)
    : TcpReactor(loop, local, heartbeat)
{
    acceptor_->setNewConnectionCallback(std::bind(&TcpSubReactor::newConnection, this, _1, _2, _3));
}

void TcpSubReactor::start() {
    acceptor_->listen();
}

void TcpSubReactor::newConnection(int connfd, const InetAddress& local, const InetAddress& peer) {
    loop_->assertInLoopThread();

    auto connPtr = std::make_shared<TcpConnection>(loop_, connfd, local, peer, heartbeat_);
    connections_.insert(connPtr);

    connPtr->setMessageCallback(messageCallback_);
    connPtr->setWriteCompleteCallback(writeCompleteCallback_);
    connPtr->setCloseCallback(std::bind(&TcpSubReactor::closeConnection, this, _1));
    connPtr->setConnectionCallback(connectionCallback_);

    loop_->runInLoop(std::bind(&TcpConnection::connectionEstablished, connPtr));
}

void TcpSubReactor::closeConnection(const TcpConnectionPtr& connPtr) {
    loop_->runInLoop(std::bind(&TcpSubReactor::closeConnectionInLoop, this, connPtr));
}

void TcpSubReactor::closeConnectionInLoop(const TcpConnectionPtr &connPtr) {
    loop_->assertInLoopThread();
    auto ret = connections_.erase(connPtr);
    assert(ret == 1);(void)ret;
    // connPtr->forceClose();
    loop_->queueInLoop(std::bind(&TcpConnection::connectionDestroyed, connPtr));
}
