#include "TcpClient.h"
#include "Callbacks.h"
#include "Connector.h"
#include "InetAddress.h"
#include "libnet/base/Logger.h"
#include "TcpConnection.h"
#include <cassert>
#include <memory>

using namespace libnet;

TcpClient::TcpClient(EventLoop* loop, const InetAddress& peer)
    : loop_(loop),
      connected_(false),
      peer_(peer),
      retryTimer_(nullptr),
      connector_(std::make_unique<Connector>(loop, peer)),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback)
{
    connector_->setNewConnectionCallback([this](int connfd, const InetAddress& l, const InetAddress& p)
                                        {
                                            this->newConnection(connfd, l, p);
                                        });
}

TcpClient::~TcpClient()
{
    if (connection_ && !connection_->disconnected()) {
        connection_->forceClose();
    }
    if (retryTimer_ != nullptr) {
        loop_->cancelTimer(retryTimer_);
    }
}

void TcpClient::start() {
    loop_->assertInLoopThread();
    connector_->start();
    retryTimer_ = loop_->runEvery(3s, [this]() { retry(); });
}

void libnet::TcpClient::retry() {
    loop_->assertInLoopThread();
    if (connected_) {
        return;
    }

    LOG_WARN << "TcpClient::retry() reconnecting " << peer_.toIpPort() << "...";

    connector_ = std::make_unique<Connector>(loop_, peer_);
    connector_->setNewConnectionCallback([this](auto connfd, auto local, auto peer)
                                        {
                                            this->newConnection(connfd, local, peer);
                                        });
    connector_->start();
}

void TcpClient::newConnection(int connfd, const InetAddress& local, const InetAddress& peer) {
    loop_->assertInLoopThread();
    loop_->cancelTimer(retryTimer_);
    retryTimer_ = nullptr;
    connected_ = true;

    auto conn = std::make_shared<TcpConnection>(loop_, connfd, local, peer, 10s);
    connection_ = conn;
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback([this](auto connptr)
                            {
                                this->closeConnection(connptr);
                            });
    conn->connectionEstablished();
    connectionCallback_(conn);
}

void TcpClient::closeConnection(const TcpConnectionPtr& conn) {
    loop_->assertInLoopThread();
    assert(connection_ != nullptr);
    connection_.reset();
    connectionCallback_(conn);
}

