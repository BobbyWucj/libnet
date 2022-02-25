#ifndef LIBNET_TIMERQUEUE_H
#define LIBNET_TIMERQUEUE_H

#include <any>
#include <chrono>
#include <cstdint>
#include <memory>
#include <vector>
#include <queue>
#include <deque>
#include "Timer.h"
#include "Channel.h"
#include "Timestamp.h"

namespace libnet
{

struct TimerCmp {
    bool operator()(Timer* lhs, Timer* rhs) {
        return lhs->when() > rhs->when();
    }
};

class TimerQueue: noncopyable
{
public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    Timer* addTimer(TimerCallback cb, Timestamp when, Nanoseconds interval = Milliseconds::zero(), bool repeat = false);

    void cancelTimer(Timer* timer);

    void updateTimer(Timer* timer, Timestamp when);

    int64_t nextTimeout() const {
        if (timers_.empty()) {
            return 0;
        }
        auto interval = timers_.top()->when() - clock::now();
        if (interval.count() < 0) {
            return 0;
        }
        return std::chrono::duration_cast<Milliseconds>(interval).count();
    }

private:
    using TimerHeap = std::priority_queue<Timer*, std::deque<Timer*>, TimerCmp>;

    void handleRead();

    EventLoop* loop_;
    const int timerfd_;
    Channel timerChannel_;
    TimerHeap timers_;
};


} // namespace libnet


#endif // LIBNET_TIMERQUEUE_H
