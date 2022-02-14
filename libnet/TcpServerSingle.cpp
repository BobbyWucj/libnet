#include "Callbacks.h"
#include "EventLoop.h"
#include "TcpServerSingle.h"
#include "TcpConnection.h"
#include <cassert>

using namespace libnet;

TcpServerSingle::TcpServerSingle(EventLoop* loop, const InetAddress& local)
    : loop_(loop),
      acceptor_(std::make_unique<Acceptor>(loop, local))
{
    acceptor_->setNewConnectionCallback([this](int connfd, const InetAddress& localAddr, const InetAddress& peerAddr)
                                        { this->newConnection(connfd, localAddr, peerAddr); });
}

void TcpServerSingle::start() {
    acceptor_->listen();
}

void TcpServerSingle::newConnection(int connfd, const InetAddress& local, const InetAddress& peer) {
    loop_->assertInLoopThread();
    auto connPtr = std::make_shared<TcpConnection>(loop_, connfd, local, peer);
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
    assert(connections_.erase(connPtr) == 1);
}
