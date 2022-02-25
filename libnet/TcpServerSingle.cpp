#include "Callbacks.h"
#include "EventLoop.h"
#include "TcpServerSingle.h"
#include "TcpConnection.h"
#include "libnet/Timestamp.h"
#include "libnet/base/Logger.h"
#include <any>
#include <cassert>
#include <functional>

using namespace libnet;


TcpServerSingle::TcpServerSingle(EventLoop* loop, const InetAddress& local, const Nanoseconds heartbeat)
    : loop_(loop),
      acceptor_(std::make_unique<Acceptor>(loop, local)),
      heartbeat_(heartbeat)
{
    acceptor_->setNewConnectionCallback(std::bind(&TcpServerSingle::newConnection, this, _1, _2, _3));
}

void TcpServerSingle::start() {
    acceptor_->listen();
}

void TcpServerSingle::newConnection(int connfd, const InetAddress& local, const InetAddress& peer) {
    loop_->assertInLoopThread();
    auto connPtr = std::make_shared<TcpConnection>(loop_, connfd, local, peer, heartbeat_);
    connections_.insert(connPtr);

    connPtr->setMessageCallback(messageCallback_);
    connPtr->setWriteCompleteCallback(writeCompleteCallback_);
    connPtr->setCloseCallback([this](const TcpConnectionPtr& conn) {
                                this->closeConnection(conn);
                            });
    connPtr->connectionEstablished();
    connectionCallback_(connPtr);
}

void TcpServerSingle::closeConnection(const TcpConnectionPtr& connPtr) {
    loop_->assertInLoopThread();
    auto ret = connections_.erase(connPtr);
    assert(ret == 1);(void)ret;
}