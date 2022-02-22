#include "TcpServer.h"
#include "Logger.h"
#include "EventLoop.h"
#include "TcpServerSingle.h"


using namespace libnet;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& local)
    : baseLoop_(loop),
      numThreads_(1),
      started_(false),
      local_(local),
      ipPort_(local.toIpPort()),
      threadInitCallback_(defaultThreadInitCallback),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback)
{
    LOG_TRACE("Create TcpServer() %s", local.toIpPort().c_str());
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
    LOG_TRACE("~TcpServer()");
}

void TcpServer::setNumThreads(size_t numThreads) {
    baseLoop_->assertInLoopThread();
    assert(numThreads > 0);
    assert(!started_);
    numThreads_ = numThreads;
    eventLoops_.resize(numThreads);
}

void TcpServer::start() {
    if (started_.exchange(true)) {
        return;
    }
    baseLoop_->runInLoop([this]() 
                        {
                            this->startInLoop();
                        });
}

void TcpServer::startInLoop() {
    LOG_INFO("TcpServer::start() %s with %lu eventLOop thread(s)", 
            local_.toIpPort().c_str(), numThreads_);
    
    baseServer_ = std::make_unique<TcpServerSingle>(baseLoop_, local_);
    baseServer_->setConnectionCallback(connectionCallback_);
    baseServer_->setMessageCallback(messageCallback_);
    baseServer_->setWriteCompleteCallback(writeCompleteCallback_);

    // main thread
    threadInitCallback_(0);
    baseServer_->start();

    // create numThreads-1 threads and loop
    for (size_t i = 1; i < numThreads_; ++i) {
        auto thread = new std::thread([this, i]()
                                     {
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

void TcpServer::runInThread(const size_t index) {
    EventLoop loop;
    // 栈上对象
    TcpServerSingle server(&loop, local_);

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
