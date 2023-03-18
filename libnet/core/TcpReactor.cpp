/*
 * TcpReactor.cpp
 * Created on 2023/3/3
 * Copyright (c) 2023 BobbyWucj
 *
 *
 */

#include "core/TcpReactor.h"
#include "core/EventLoop.h"
#include "core/TcpConnection.h"
#include <memory>

using namespace libnet;

TcpReactor::TcpReactor(EventLoop* loop, const InetAddress& local, const Nanoseconds heartbeat)
    : loop_(loop),
      acceptor_(std::make_unique<Acceptor>(loop, local)),
      connections_(),
      connectionCallback_(),
      messageCallback_(),
      writeCompleteCallback_(),
      heartbeat_(heartbeat)
{
}

TcpReactor::~TcpReactor()
{
    for(auto &item : connections_)
    {
        TcpConnectionPtr conn(item);
        // 把原始的智能指针复位 让栈空间的TcpConnectionPtr conn指向该对象
        // 当conn出了其作用域 即可释放智能指针指向的对象
        connections_.erase(item);
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::forceClose, conn));
    }
}
