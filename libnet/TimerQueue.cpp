
#include <sys/timerfd.h>
#include <strings.h>
#include <unistd.h>
#include <ratio> // for std::nano::den

#include "Logger.h"
#include "EventLoop.h"
#include "TimerQueue.h"

using namespace libnet;

namespace
{

int timerfdCreate() {
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if(fd == -1)
        LOG_SYSFATAL("timerfd_create()");
    return fd;
}

void timerfdRead(int fd)
{
  uint64_t val;
  ssize_t n = ::read(fd, &val, sizeof(val));
  if (n != sizeof(val))
    LOG_ERROR("timerfdRead get %ld, not %lu", n, sizeof(val));
}

struct timespec durationFromNow(Timestamp when) {
    struct timespec ret;
    Nanoseconds ns = when - clock::now();
    if (ns < 1ms)
        ns = 1ms;
    ret.tv_sec = static_cast<time_t>(ns.count() / std::nano::den);
    ret.tv_nsec = static_cast<long>(ns.count() % std::nano::den);
    return ret;
}

void timerfdSet(int fd, Timestamp when) {
    struct itimerspec oldtime, newtime;
    bzero(&oldtime, sizeof(itimerspec));
    bzero(&newtime, sizeof(itimerspec)); 
    newtime.it_value = durationFromNow(when);

    int ret = timerfd_settime(fd, 0, &newtime, &oldtime);
    if(ret == -1) 
        LOG_SYSERR("timerfd_settime()");
}

} // anonymous namespace

TimerQueue::TimerQueue(EventLoop* loop)
    : loop_(loop),
      timerfd_(timerfdCreate()),
      timerChannel_(loop_, timerfd_)
{
    loop_->assertInLoopThread();
    timerChannel_.setReadCallback([this]{handleRead();});
    timerChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
    for (auto& entry : timers_)
        delete entry.second;
    ::close(timerfd_);
}

Timer* TimerQueue::addTimer(TimerCallback cb, Timestamp when, Nanoseconds interval) {
    Timer* timer = new Timer(std::move(cb), when, interval);
    loop_->runInLoop([=] {
                        auto ret = timers_.insert({when, timer});
                        assert(ret.second);
                        if(timers_.begin() == ret.first) {
                            timerfdSet(timerfd_, when);
                        }
                    });
    return timer;
}

void TimerQueue::cancelTimer(Timer* timer) {
    loop_->runInLoop([timer, this]
                     {
                         timer->cancel();
                         timers_.erase({timer->when(), timer});
                         delete timer;
                     });
}

void TimerQueue::handleRead() {
    loop_->assertInLoopThread();
    timerfdRead(timerfd_);
    
    Timestamp now(clock::now());

    for(auto& entry : getExpired(now)) {
        Timer* timer = entry.second;
        assert(timer->expired(now));

        if (!timer->canceled()) {
            timer->run();
        }
        if (!timer->canceled() && timer->repeat()) {
            timer->restart();
            entry.first = timer->when();
            timers_.insert(entry);
        } else {
            delete timer;
        }
    }
    if (!timers_.empty())
        timerfdSet(timerfd_, timers_.begin()->first);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
    // 1ns is the precision
    Entry entry(now + 1ns, nullptr);
    // find the first timer which not expired
    auto end = timers_.lower_bound(entry);
    assert(end == timers_.end() || now < end->first);
    std::vector<Entry> expired(timers_.begin(), end);
    timers_.erase(timers_.begin(), end);

    return expired;
}


