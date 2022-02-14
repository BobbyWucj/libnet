#include <cassert>
#include <cerrno>
#include <fcntl.h>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "Acceptor.h"
#include "InetAddress.h"
#include "Logger.h"
#include "EventLoop.h"

using namespace libnet;

namespace {

int createSocket() {
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd == -1) {
        LOG_SYSFATAL("Acceptor::createSocket()");
    }
    return sockfd;
}

} // anonymous namespace

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr)
    : listening_(false),
      listenFd_(createSocket()),
      emfileFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)),
      loop_(loop),
      listenChannel_(std::make_unique<Channel>(loop, listenFd_)),
      listenAddr_(listenAddr),
      newConnectionCallback_(nullptr)
{
    assert(emfileFd_ > 0);
    assert(listenFd_ > 0);
    assert(loop_);

    int on = 1;
    int ret = ::setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (ret == -1) {
        LOG_SYSFATAL("Acceptor::setsocketopt() SO_REUSEADDRESS");
    }
    ret = ::setsockopt(listenFd_, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
    if (ret == -1) {
        LOG_SYSFATAL("Acceptor::setsocketopt() SO_REUSEPORT");
    }
    ret = ::bind(listenFd_, listenAddr_.getSockaddr(), listenAddr_.getSocklen());
    if (ret == -1) {
        LOG_SYSFATAL("Acceptor::bind()");
    }
}

Acceptor::~Acceptor()
{
    assert(listening_);
    listening_ = false;
    listenChannel_->disableReading();
    if (listenFd_) {
        ::close(listenFd_);
    }
    if (emfileFd_) {
        ::close(emfileFd_);
    }
}

void Acceptor::listen() {
    listening_ = true;
    loop_->assertInLoopThread();
    int ret = ::listen(listenFd_, SOMAXCONN - 1);
    if (ret == -1) {
        LOG_SYSFATAL("Acceptor::listen()");
    }
    listenChannel_->setReadCallback([this]{ this->handleRead(); });
    listenChannel_->enableReading();
}

void Acceptor::handleRead() {
    loop_->assertInLoopThread();
    struct sockaddr_in address;
    socklen_t len = sizeof(address);
    int cfd = -1;
    while (1) {
        cfd = ::accept4(listenFd_, reinterpret_cast<struct sockaddr*>(&address), &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (cfd == -1) {
            switch (errno) {
            case EWOULDBLOCK:
            case EINTR:
                continue;
            case EMFILE: // 当前进程打开的文件描述符已达上限
                ::close(emfileFd_);
                emfileFd_ = ::accept(listenFd_, nullptr, nullptr);
                ::close(emfileFd_);
                emfileFd_ = ::open("dev/null", O_CLOEXEC | O_RDONLY);
                continue;
            default:
                LOG_FATAL("unexpected accept4() error");
            }
        } else {
            break;
        }
    }
    if (newConnectionCallback_) {
        InetAddress peerAddr;
        peerAddr.setAddress(address);
        newConnectionCallback_(cfd, listenAddr_, peerAddr);
    } else {
        ::close(cfd);
    }
}

