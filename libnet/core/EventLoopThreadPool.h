#ifndef LIBNET_EVENTLOOPTHREADPOOL_H
#define LIBNET_EVENTLOOPTHREADPOOL_H

#include <cstddef>
#include <memory>
#include <vector>

#include "core/EventLoopThread.h"
#include "logger/Logger.h"

namespace libnet {

class EventLoop;

class EventLoopThreadPool
{
public:
    using ptr = std::shared_ptr<EventLoopThreadPool>;

    EventLoopThreadPool(EventLoop* baseLoop, size_t numThreads);
    ~EventLoopThreadPool() { LOG_INFO << "~EventLoopThreadPool()"; }

    void start();

    EventLoop* getNextLoop();
    size_t     numThreads() const { return numThreads_; }

private:
    EventLoop*                                    baseLoop_;
    bool                                          started_;
    size_t                                        numThreads_;
    size_t                                        next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*>                       loops_;
};

}  // namespace libnet

#endif  // LIBNET_EVENTLOOPTHREADPOOL_H
