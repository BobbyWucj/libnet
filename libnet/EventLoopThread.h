#ifndef LIBNET_EVENTLOOPTHREAD_H
#define LIBNET_EVENTLOOPTHREAD_H

#include <thread>
#include "CountDownLatch.h"

namespace libnet
{
    
class EventLoop;

class EventLoopThread : noncopyable
{
public:
    EventLoopThread();
    ~EventLoopThread();

    EventLoop* startLoop();

private:
    void runInThread();

    bool            started_;
    EventLoop*      loop_;
    std::thread     thread_;
    CountDownLatch  latch_;
};


} // namespace libnet


#endif // LIBNET_EVENTLOOPTHREAD_H
