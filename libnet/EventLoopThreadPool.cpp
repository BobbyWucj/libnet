#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "libnet/EventLoopThread.h"
#include "libnet/base/Logger.h"
#include <cstddef>
#include <memory>

using namespace libnet;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, size_t numThreads)
    : baseLoop_(baseLoop),
      started_(false),
      numThreads_(numThreads),
      next_(0)
{
    if (numThreads <= 0) {
        LOG_SYSFATAL << "EventLoopThreadPool() : numThreads <= 0";
    }
}

void EventLoopThreadPool::start() {
    baseLoop_->assertInLoopThread();
    started_ = true;
    for (size_t i = 0; i < numThreads_; ++i) {
        EventLoopThread* t = new EventLoopThread();
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop());
    }
}

EventLoop* EventLoopThreadPool::getNextLoop() {
    baseLoop_->assertInLoopThread();
    assert(started_);
    EventLoop *loop = baseLoop_;
    // round-robin
    if (!loops_.empty()) {
        loop = loops_[next_];
        next_ = (next_ + 1) % numThreads_;
    }
    return loop;
}
