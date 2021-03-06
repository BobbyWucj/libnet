#include "TcpConnection.h"
#include "libnet/TcpServerSingle.h"
#include "libnet/Timestamp.h"
#include "libnet/base/Logger.h"
#include "EventLoop.h"

#include <cassert>
#include <cerrno>
#include <cstddef>
#include <unistd.h>

using namespace libnet;

namespace libnet {

void defaultThreadInitCallback(size_t index) {
    LOG_TRACE << "EventLoop thread " << index << " started";
}

void defaultConnectionCallback(const TcpConnectionPtr &conn) {
    LOG_TRACE << conn->local().toIpPort() << " -> "
              << conn->peer().toIpPort() << " is "
              << (conn->connected() ? "UP" : "DOWN");
}

void defaultMessageCallback(const TcpConnectionPtr& conn, Buffer& buffer)
{
    LOG_TRACE << conn->local().toIpPort() << " -> "
              << conn->peer().toIpPort() << " recv "
              << buffer.readableBytes() << " bytes";
    buffer.retrieveAll();
}

} // namespace libnet

TcpConnection::TcpConnection(EventLoop* loop,
                             int cfd,
                             const InetAddress& local,
                             const InetAddress& peer,
                             const Nanoseconds heartbeat)
    : loop_(loop),
      state_(kConnecting),
      cfd_(cfd),
      channel_(std::make_unique<Channel>(loop, cfd)),
      local_(std::make_unique<InetAddress>(local)),
      peer_(std::make_unique<InetAddress>(peer)),
      inputBuffer_(std::make_unique<Buffer>()),
      outputBuffer_(std::make_unique<Buffer>()),
      heartbeat_(heartbeat),
      highWaterMark_(0)
{
    channel_->setReadCallback([this]{ this->handleRead(); });
    channel_->setWriteCallback([this]{ this->handleWrite(); });
    channel_->setCloseCallback([this]{ this->handleClose(); });
    channel_->setErrorCallback([this]{ this->handleError(); });

    LOG_TRACE << "TcpConnection() " << name() << " fd=" << cfd;

    timer_ = loop_->runAfter(heartbeat_, 
                             std::bind(&TcpConnection::onInactiveConn, this));
}

TcpConnection::~TcpConnection()
{
    assert(state_ == kDisconnected);
    ::close(cfd_);
    loop_->cancelTimer(timer_);
    LOG_TRACE << "~TcpConnection() " << name() << " fd=" << cfd_;
}

void TcpConnection::connectionEstablished() {
    auto old_state = state_.exchange(kConnected);
    assert(old_state == kConnecting);(void)old_state;
    channel_->tie(shared_from_this());
    channel_->enableReading();

    connectionCallback_(shared_from_this());
}

void TcpConnection::send(const std::string& data) {
    send(data.data(), static_cast<size_t>(data.size()));
}

// thread-safe send
void TcpConnection::send(const char* data, size_t len) {
    if (state_ != kConnected) {
        LOG_WARN << "TcpConnection::send() not connected, give up send ";
        return;
    }
    if (loop_->isInLoopThread()) {
        sendInLoop(data, len);
    } else {
        loop_->queueInLoop([this, str = std::string(data, data + len)] {
                                this->sendInLoop(str);
                            });
    }
}

void TcpConnection::send(Buffer& buffer) {
    if (state_ != kConnected) {
        LOG_WARN << "TcpConnection::send() not connected, give up send ";
        return;
    }
    if (loop_->isInLoopThread()) {
        sendInLoop(buffer.peek(), buffer.readableBytes());
        buffer.retrieveAll();
    } else {
        loop_->queueInLoop([this, str = buffer.retrieveAllAsString()] {
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
        LOG_WARN << "TcpConnection::sendInLoop() disconnected, give up send";
        return;
    }
    ssize_t n = 0;
    size_t remain = len;
    bool faultError = false;

    // ??????????????????????????????????????????????????????????????????????????????
    if (!channel_->isWriting() && outputBuffer_->readableBytes() == 0) {
        n = ::write(cfd_, data, len);
        if (n == -1) {
            if (errno != EWOULDBLOCK || errno != EINTR || errno != EAGAIN) {
                LOG_SYSERR << "TcpConnection::write()";
                // remote closed
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
            n = 0;
        } else {
            remain -= static_cast<size_t>(n);
            if (remain == 0 && writeCompleteCallback_) {
                loop_->queueInLoop([this] {
                                        this->writeCompleteCallback_(this->shared_from_this());
                                    });
            }
        }
    }
    // still remain
    if (!faultError && remain > 0) {
        if (highWaterMarkCallback_) {
            size_t oldLen = outputBuffer_->readableBytes();
            size_t newLen = oldLen + remain;
            // ?????????????????????
            if (oldLen < highWaterMark_ && newLen >= highWaterMark_) {
                loop_->queueInLoop([this, &newLen] {
                                        this->highWaterMarkCallback_(this->shared_from_this(), newLen);
                                    });
            }
        }
        // ???????????????????????? outputbuffer
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
            loop_->queueInLoop(std::bind(
                    &TcpConnection::shutdownInLoop, shared_from_this()));
        }
    }
}

void TcpConnection::shutdownInLoop() {
    loop_->assertInLoopThread();

    if (state_ != kDisconnected && !channel_->isWriting()) {
        if (::shutdown(cfd_, SHUT_WR) == -1) {
            LOG_SYSERR << "TcpConnection::shutdown()";
        }
    }
}

void TcpConnection::forceClose() {
    if (state_ != kDisconnected && state_.exchange(kDisconnecting) != kDisconnected) {
        loop_->queueInLoop(std::bind(
                    &TcpConnection::forceCloseInLoop, shared_from_this()));
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
    timer_->setWhen(clock::now() + timer_->interval());
    ssize_t n = inputBuffer_->readFd(cfd_, &savedErrno);
    if (n == -1) {
        errno = savedErrno;
        LOG_SYSERR << "TcpConnection::handleRead()";
        handleError();
    } else if (n == 0) {
        handleClose();
    } else {
        messageCallback_(shared_from_this(), *inputBuffer_);
    }
}

void TcpConnection::handleWrite() {
    if (state_ == kDisconnected) {
        LOG_WARN << "TcpConnection::handleWrite() disconnected, " 
                 << "give up writing " << outputBuffer_->readableBytes() << " bytes";
        return;
    }
    assert(outputBuffer_->readableBytes() > 0);
    assert(channel_->isWriting());
    timer_->setWhen(clock::now() + timer_->interval());
    ssize_t n = ::write(cfd_, outputBuffer_->peek(), outputBuffer_->readableBytes());
    if (n == -1) {
        LOG_SYSERR << "TcpConnection::write()";
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
    auto old_state = state_.exchange(kDisconnected);
    assert(old_state <= kDisconnecting);(void)old_state;
    loop_->removeChannel(channel_.get());
    
    connectionCallback_(this->shared_from_this());
    closeCallback_(this->shared_from_this());
}

void TcpConnection::handleError() {
    int err = 0;
    socklen_t len = sizeof(err);
    int ret = getsockopt(cfd_, SOL_SOCKET, SO_ERROR, &err, &len);
    if (ret != -1) {
        errno = err;
    }
    LOG_SYSERR << "TcpConnection::handleError()" << errno;
}

void TcpConnection::onInactiveConn() {
    if (timer_->expired(clock::now())) {
        handleClose();
        LOG_INFO << "Connection " << name() << " timeout! Shutdown now!";
    }
}

