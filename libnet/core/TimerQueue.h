#ifndef LIBNET_TIMERQUEUE_H
#define LIBNET_TIMERQUEUE_H

#include "core/Channel.h"
#include "core/Timer.h"
#include "core/Timestamp.h"
#include <any>
#include <chrono>
#include <cstdint>
#include <deque>
#include <memory>
#include <queue>
#include <vector>

namespace libnet {

struct TimerCmp
{
    bool operator()(Timer::sptr lhs, Timer::sptr rhs) {
        return lhs->when() > rhs->when();
    }
};

class TimerQueue : noncopyable
{
public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    Timer::sptr addTimer(TimerCallback cb,
                         Timestamp     when,
                         Nanoseconds   interval = Milliseconds::zero(),
                         bool          repeat   = false);

    void cancelTimer(Timer::sptr timer);

    void updateTimer(Timer::sptr timer, Timestamp when);

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

    int timerfd() const { return timerfd_; }

private:
    using TimerHeap =
        std::priority_queue<Timer::sptr, std::vector<Timer::sptr>, TimerCmp>;

    void handleRead();

    EventLoop* loop_;
    const int  timerfd_;
    Channel    timerChannel_;
    TimerHeap  timers_;
};

}  // namespace libnet

#endif  // LIBNET_TIMERQUEUE_H
