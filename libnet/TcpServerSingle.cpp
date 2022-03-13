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
      heartbeat_(heartbeat),
      reusePort_(true),
      threadPool_(nullptr)
{
    acceptor_->setNewConnectionCallback(std::bind(&TcpServerSingle::newConnection, this, _1, _2, _3));
}

void TcpServerSingle::start() {
    acceptor_->listen();
    if (!reusePort_) {
        threadPool_->start();
    }
}

void TcpServerSingle::newConnection(int connfd, const InetAddress& local, const InetAddress& peer) {
    EventLoop* ioLoop = loop_;
    if (!reusePort_ && threadPool_) {
        ioLoop = threadPool_->getNextLoop();
    }
    ioLoop->assertInLoopThread();
    auto connPtr = std::make_shared<TcpConnection>(ioLoop, connfd, local, peer, heartbeat_);
    connections_.insert(connPtr);

    connPtr->setMessageCallback(messageCallback_);
    connPtr->setWriteCompleteCallback(writeCompleteCallback_);
    connPtr->setCloseCallback([this](const TcpConnectionPtr& conn) {
                                this->closeConnection(conn);
                            });
    ioLoop->runInLoop(std::bind(&TcpConnection::connectionEstablished, connPtr));
    connectionCallback_(connPtr);
}

void TcpServerSingle::closeConnection(const TcpConnectionPtr& connPtr) {
    loop_->assertInLoopThread();
    auto ret = connections_.erase(connPtr);
    assert(ret == 1);(void)ret;
    connPtr->forceClose();
}

// for reusePort == false
// 0 - [default value].means all I/O in baseloop's thread, no thread will created.
// 1 - means all I/O in another thread.
// N - a EventLoopThreadPool with N threads, 
//     new connections are assigned on a round-robin basis.
void TcpServerSingle::disableReusePort(size_t numThreads) {
    reusePort_ = false;
    threadPool_ = std::make_unique<EventLoopThreadPool>(loop_, numThreads);
}

size_t TcpServerSingle::threadNum() const {
    if(!threadPool_) {
        return 0;
    }
    return threadPool_->numThreads();
}
