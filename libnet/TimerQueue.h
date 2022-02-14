#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H

#include <chrono>
#include <cstdint>
#include <memory>
#include <set>
#include <vector>

#include "Timer.h"
#include "Channel.h"
#include "Timestamp.h"

namespace libnet
{

class TimerQueue: noncopyable
{
public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    Timer* addTimer(TimerCallback cb, Timestamp when, Nanoseconds interval);

    void cancelTimer(Timer* timer);

    int64_t nextTimeout() const {
        if (timers_.empty()) {
            return 0;
        }
        auto interval = timers_.begin()->first - clock::now();
        return std::chrono::duration_cast<Milliseconds>(interval).count();
    }

private:
    using Entry = std::pair<Timestamp, Timer*>;
    using TimerList = std::set<Entry>;

    void handleRead();
    std::vector<Entry> getExpired(Timestamp now);

    EventLoop* loop_;
    const int timerfd_;
    Channel timerChannel_;
    TimerList timers_;
};


} // namespace libnet


#endif // TIMERQUEUE_H
