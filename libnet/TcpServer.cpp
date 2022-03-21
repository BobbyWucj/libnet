#include "TcpServer.h"
#include "libnet/EventLoopThreadPool.h"
#include "libnet/base/Logger.h"
#include "EventLoop.h"
#include "TcpServerSingle.h"
#include "TcpConnection.h"
#include <memory>


using namespace libnet;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& local, const Nanoseconds heartbeat)
    : baseLoop_(loop),
      numThreads_(1),
      started_(false),
      local_(local),
      ipPort_(local.toIpPort()),
      heartbeat_(heartbeat),
      reusePort_(true),
      threadInitCallback_(defaultThreadInitCallback),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback)
{
    LOG_TRACE << "Creating TcpServer() " << local.toIpPort();
}

TcpServer::~TcpServer()
{
    for (auto& loop : eventLoops_) {
        if (loop != nullptr) {
            loop->quit();
        }
    }
    for (auto& thread : threads_) {
        thread->join();
    }
    LOG_TRACE << "~TcpServer() " << local_.toIpPort();
}

void TcpServer::setNumThreads(size_t numThreads) {
    baseLoop_->assertInLoopThread();
    assert(numThreads > 0);
    assert(!started_);
    numThreads_ = numThreads;
    if (reusePort_) {
        eventLoops_.resize(numThreads);
    }
}

void TcpServer::start() {
    if (started_.exchange(true)) {
        return;
    }
    baseLoop_->runInLoop([this]() {
                        this->startInLoop();
                    });
}

void TcpServer::startInLoop() {
    LOG_INFO << "TcpServer::start() " << local_.toIpPort() << " with " << numThreads_ << " eventLoop thread(s)";
    
    baseServer_ = std::make_unique<TcpServerSingle>(baseLoop_, local_, heartbeat_);

    if (!reusePort_) {
        baseServer_->disableReusePort(numThreads_);
    }

    baseServer_->setConnectionCallback(connectionCallback_);
    baseServer_->setMessageCallback(messageCallback_);
    baseServer_->setWriteCompleteCallback(writeCompleteCallback_);

    // main thread
    threadInitCallback_(0);
    // if reusePort == false
    // baseServer_ will create numThreads-1 threads and loop
    // only baseServer_ own an acceptor
    // when new-connection comed, 
    // baseServer_ will distribute TcpConnection to EventLoopThreadPool with Round-Robin
    baseServer_->start();

    // reusePort == true, will create numThreads-1 threads and loop
    // every threads(loop) own an acceptor
    // and every threads is listening on the same port
    // The kernel will load-balance new connections to each thread
    if (reusePort_) {
        for (size_t i = 1; i < numThreads_; ++i) {
            auto thread = new std::thread([this, i]() {
                                            this->runInThread(i);
                                        });
            {
                std::unique_lock<std::mutex> lock(mutex_);
                while (eventLoops_[i] == nullptr)
                    cond_.wait(lock);
            }
            threads_.emplace_back(thread);
        }
    }
}

// only called when reusePort_ == true
void TcpServer::runInThread(const size_t index) {
    EventLoop loop;
    // stack variable
    TcpServerSingle server(&loop, local_, heartbeat_);

    server.setConnectionCallback(connectionCallback_);
    server.setMessageCallback(messageCallback_);
    server.setWriteCompleteCallback(writeCompleteCallback_);

    {
        std::lock_guard<std::mutex> guard(mutex_);
        eventLoops_[index] = &loop;
        cond_.notify_one();
    }

    threadInitCallback_(index);
    server.start();
    loop.loop();

    // stop looping
    eventLoops_[index] = nullptr;
}
