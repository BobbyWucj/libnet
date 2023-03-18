#ifndef LIBNET_EVENTLOOP_H
#define LIBNET_EVENTLOOP_H

#include <any>
#include <atomic>
#include <cstddef>
#include <thread>
#include <mutex>
#include <vector>
#include <sys/types.h>
#include "utils/noncopyable.h"
#include "core/TimerQueue.h"

namespace libnet
{

class EPoller;
class Channel;

class EventLoop : noncopyable
{
public:
    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    void runInLoop(const Task& task);
    void runInLoop(Task&& task);
    void queueInLoop(const Task& task);
    void queueInLoop(Task&& task);

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

    static EventLoop* getEventLoopOfCurrentThread();

    void assertInLoopThread() const {
        assert(isInLoopThread());
    }

    void assertNotInLoopThread() const {
        assert(!isInLoopThread());
    }

    bool isInLoopThread() const;

    Timer::sptr runAt(Timestamp when, TimerCallback callback);
    Timer::sptr runAfter(Nanoseconds interval, TimerCallback callback);
    Timer::sptr runEvery(Nanoseconds interval, TimerCallback callback);
    void cancelTimer(Timer::sptr timer);

    void wakeup();
private:
    using ChannelList = std::vector<Channel*>;
    using TaskList = std::vector<Task>;

    void doPendingTasks();
    void handleRead();

    const std::thread::id       tid_;
    std::atomic<bool>           quit_;
    std::unique_ptr<EPoller>    poller_;
    ChannelList                 activeChannels_;
    TimerQueue                  timerQueue_;
    bool                        doingPendingTasks_;
    TaskList                    pendingTasks_;
    const int                   wakeupFd_;
    std::unique_ptr<Channel>    wakeupChannel_;
    mutable std::mutex          mutex_;
};

}

#endif // LIBNET_EVENTLOOP_H
