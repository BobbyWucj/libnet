#include "TcpConnection.h"
#include "Logger.h"
#include "EventLoop.h"

#include <cassert>
#include <cstddef>
#include <unistd.h>

using namespace libnet;

namespace libnet {

void defaultThreadInitCallback(size_t index) {
    LOG_TRACE("EventLoop thread #%lu started", index);
}

void defaultConnectionCallback(const TcpConnectionPtr &conn) {
    LOG_INFO("connection %s -> %s %s",
        conn->peer().toIpPort().c_str(),
        conn->local().toIpPort().c_str(),
        conn->connected() ? "up" : "down");
}

void defaultMessageCallback(const TcpConnectionPtr& conn, Buffer& buffer)
{
  LOG_TRACE("connection %s -> %s recv %lu bytes",
        conn->peer().toIpPort().c_str(),
        conn->local().toIpPort().c_str(),
        buffer.readableBytes());
  buffer.retrieveAll();
}

} // namespace libnet

TcpConnection::TcpConnection(EventLoop* loop,
                             int cfd,
                             const InetAddress& local,
                             const InetAddress& peer)
    : loop_(loop),
      state_(kConnecting),
      cfd_(cfd),
      channel_(std::make_unique<Channel>(loop, cfd)),
      local_(std::make_unique<InetAddress>(local)),
      peer_(std::make_unique<InetAddress>(peer)),
      inputBuffer_(std::make_unique<Buffer>()),
      outputBuffer_(std::make_unique<Buffer>()),
      highWaterMark_(0)
{
    channel_->setReadCallback([this]{ this->handleRead(); });
    channel_->setWriteCallback([this]{ this->handleWrite(); });
    channel_->setCloseCallback([this]{ this->handleClose(); });
    channel_->setErrorCallback([this]{ this->handleError(); });

    LOG_TRACE("TcpConnection() %s fd=%d", name().c_str(), cfd);
}

TcpConnection::~TcpConnection()
{
    assert(state_ == kDisconnected);
    ::close(cfd_);

    LOG_TRACE("~TcpConnection() %s fd=%d", name().c_str(), cfd_);
}

void TcpConnection::connectionEstablished() {
    assert(state_.exchange(kConnected) == kConnecting);
    channel_->tie(shared_from_this());
    channel_->enableReading();
}

void TcpConnection::send(const std::string& data) {
    send(data.data(), static_cast<size_t>(data.size()));
}

// thread-safe send
void TcpConnection::send(const char* data, size_t len) {
    if (state_ != kConnected) {
        LOG_WARN("TcpConnection::send() not connected, give up send ");
        return;
    }
    if (loop_->isInLoopThread()) {
        sendInLoop(data, len);
    } else {
        loop_->queueInLoop([this, str = std::string(data, data + len)]
                            {
                                this->sendInLoop(str);
                            });
    }

}

void TcpConnection::send(Buffer& buffer) {
    if (state_ != kConnected) {
        LOG_WARN("TcpConnection::send() not connected, give up send ");
        return;
    }
    if (loop_->isInLoopThread()) {
        sendInLoop(buffer.peek(), buffer.readableBytes());
        buffer.retrieveAll();
    } else {
        loop_->queueInLoop([this, str = buffer.retrieveAllAsString()]
                            {
                                this->sendInLoop(str);
                            });
    } 
}

void TcpConnection::sendInLoop(const std::string& message) {
    sendInLoop(message.data(), message.size());
}

void TcpConnection::sendInLoop(const char* data, size_t len) {
    loop_->assertInLoopThread();
    if (state_ == kDisconnected) {
        LOG_WARN("TcpConnection::sendInLoop() disconnected, give up send");
        return;
    }
    ssize_t n = 0;
    size_t remain = len;
    bool faultError = false;

    // 如果没有注册可写事件，输出缓冲区没有数据，则直接发送
    if (!channel_->isWriting() && outputBuffer_->readableBytes() == 0) {
        n = ::write(cfd_, data, len);
        if (n == -1) {
            if (errno != EWOULDBLOCK || errno != EINTR || errno != EAGAIN) {
                LOG_SYSERR("TcpConnection::write()");
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
            n = 0;
        } else {
            remain -= static_cast<size_t>(n);
            if (remain == 0 && writeCompleteCallback_) {
                loop_->queueInLoop([this]
                                    {
                                        this->writeCompleteCallback_(this->shared_from_this());
                                    });
            }
        }
    }

    if (!faultError && remain > 0) {
        if (highWaterMarkCallback_) {
            size_t oldLen = outputBuffer_->readableBytes();
            size_t newLen = oldLen + remain;
            if (oldLen < highWaterMark_ && newLen >= highWaterMark_) {
                loop_->queueInLoop([this, &newLen]
                                    {
                                        this->highWaterMarkCallback_(this->shared_from_this(), newLen);
                                    });
            }
        }
        // 将剩余内容添加到 outputbuffer
        outputBuffer_->append(data + n, remain);

        if (!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdown() {
    assert(state_ <= kDisconnecting);

    if (state_.exchange(kDisconnecting) == kConnected) {
        if (loop_->isInLoopThread()) {
            shutdownInLoop();
        } else {
            loop_->queueInLoop([this]
                                {
                                    this->shutdownInLoop();
                                });
        }
    }
}

void TcpConnection::shutdownInLoop() {
    loop_->assertInLoopThread();

    if (state_ != kDisconnected && !channel_->isWriting()) {
        if (::shutdown(cfd_, SHUT_WR) == -1) {
            LOG_SYSERR("TcpConnection::shutdown()");
        }
    }
}

void TcpConnection::forceClose() {
    if (state_ != kDisconnected && state_.exchange(kDisconnecting) != kDisconnected) {
        loop_->queueInLoop([this] {
                                this->forceCloseInLoop();
                          });
    }
}

void TcpConnection::forceCloseInLoop() {
    loop_->assertInLoopThread();
    if (state_ != kDisconnected) {
        handleClose();
    }
}

void TcpConnection::startRead() {
    loop_->runInLoop([this] {
                        if (!channel_->isReading()) {
                            channel_->enableReading();
                        }
                    });
}

void TcpConnection::stopRead() {
    loop_->runInLoop([this] {
                        if (channel_->isReading()) {
                            channel_->disableReading();
                        }
                    });
}

void TcpConnection::handleRead() {
    loop_->assertInLoopThread();
    assert(state_ != kDisconnected);
    int savedErrno;
    ssize_t n = inputBuffer_->readFd(cfd_, &savedErrno);
    if (n == -1) {
        errno = savedErrno;
        LOG_SYSERR("TcpConnection::read()");
        handleError();
    } else if (n == 0) {
        handleClose();
    } else {
        messageCallback_(shared_from_this(), *inputBuffer_);
    }
}

void TcpConnection::handleWrite() {
    if (state_ == kDisconnected) {
        LOG_WARN("TcpConnection::handleWrite() disconnected,"
                  "give up writing %lu bytes", outputBuffer_->readableBytes());
        return;
    }
    assert(outputBuffer_->readableBytes() > 0);
    assert(channel_->isWriting());
    ssize_t n = ::write(cfd_, outputBuffer_->peek(), outputBuffer_->readableBytes());
    if (n == -1) {
        LOG_SYSERR("TcpConnection::write()");
    } else {
        outputBuffer_->retrieve(static_cast<size_t>(n));
        if (outputBuffer_->readableBytes() == 0) {
            channel_->disableWriting();
            if (writeCompleteCallback_) {
                loop_->queueInLoop([this] {
                                        this->writeCompleteCallback_(this->shared_from_this());
                                    });
            }

            if (state_ == kDisconnecting) {
                shutdownInLoop();
            }
        }
    }
}

void TcpConnection::handleClose() {
    loop_->assertInLoopThread();
    assert(state_.exchange(kDisconnected) <= kDisconnecting);
    loop_->removeChannel(channel_.get());
    closeCallback_(this->shared_from_this());
}

void TcpConnection::handleError() {
    int err = 0;
    socklen_t len = sizeof(err);
    int ret = getsockopt(cfd_, SOL_SOCKET, SO_ERROR, &err, &len);
    if (ret != -1) {
        errno = err;
    }
    LOG_SYSERR("TcpConnection::handleError()");
}

