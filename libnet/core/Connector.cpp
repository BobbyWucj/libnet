#include "core/Connector.h"
#include "core/InetAddress.h"
#include "logger/Logger.h"
#include "core/EventLoop.h"

#include <cassert>
#include <sys/socket.h>
#include <unistd.h>

using namespace libnet;

namespace
{

int creatSocket() {
    int ret = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (ret == -1) {
        LOG_SYSFATAL << "Connector::socket()";
    }
    return ret;
}

} // anonymous namespace


Connector::Connector(EventLoop* loop, const InetAddress& peer)
    : loop_(loop),
      peer_(peer),
      cfd_(creatSocket()),
      connected_(false),
      started_(false),
      channel_(loop, cfd_)
{
    channel_.setWriteCallback([this]() { this->handleWrite(); });
}

Connector::~Connector()
{
    if (!connected_) {
        ::close(cfd_);
    }
}

void Connector::start() {
    loop_->assertInLoopThread();
    assert(!started_);
    started_ = true;

    int ret = ::connect(cfd_, peer_.getSockaddr(), peer_.getSocklen());
    if (ret == -1) {
        if (errno != EINPROGRESS) {
            handleWrite();
        } else {
            channel_.enableWriting();
        }
    } else {
        handleWrite();
    }
}

void Connector::handleWrite() {
    loop_->assertInLoopThread();
    assert(started_);

    loop_->removeChannel(&channel_);

    int err = 0;
    socklen_t len = sizeof(err);
    int ret = ::getsockopt(cfd_, SOL_SOCKET, SO_ERROR, &err, &len);
    if (ret == 0) {
        errno = err;
    }
    if (errno != 0) {
        LOG_SYSERR << "Connector::connect()";
        if (errorCallback_) {
            errorCallback_();
        }
    } else if (newConnectionCallback_) {
        struct sockaddr_in addr;
        len = sizeof(addr);
        ret = ::getsockname(cfd_, reinterpret_cast<sockaddr*>(&addr), &len);
        if (ret == -1) {
            LOG_SYSERR << "Connector::getsockname()";
        }
        InetAddress local;
        local.setAddress(addr);

        connected_ = true;
        newConnectionCallback_(cfd_, local, peer_);
    }
}

