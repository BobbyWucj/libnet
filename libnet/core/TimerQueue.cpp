#include <cassert>
#include <memory>
#include <sys/timerfd.h>
#include <strings.h>
#include <unistd.h>
#include <ratio> // for std::nano::den
#include <utility>

#include "core/Timestamp.h"
#include "logger/Logger.h"
#include "core/EventLoop.h"
#include "core/TimerQueue.h"

using namespace libnet;

namespace
{

int timerfdCreate() {
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if(fd == -1)
        LOG_SYSFATAL << "timerfd_create()";
    return fd;
}

void timerfdRead(int fd)
{
  uint64_t val;
  ssize_t n = ::read(fd, &val, sizeof(val));
  if (n != sizeof(val))
    LOG_ERROR << "timerfdRead get" << n << ", not " << sizeof(val);
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

// set timerfd expire-time
void timerfdSet(int fd, Timestamp when) {
    struct itimerspec oldtime, newtime;
    bzero(&oldtime, sizeof(itimerspec));
    bzero(&newtime, sizeof(itimerspec));
    newtime.it_value = durationFromNow(when);

    int ret = timerfd_settime(fd, 0, &newtime, &oldtime);
    if(ret == -1) 
        LOG_SYSERR << "timerfd_settime()";
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
    ::close(timerfd_);
}

Timer::sptr TimerQueue::addTimer(TimerCallback cb, Timestamp when, Nanoseconds interval, bool repeat) {
    auto timer = std::make_shared<Timer>(std::move(cb), when, interval, repeat);
    loop_->runInLoop([=] {
                        timers_.push(timer);
                        // update timerfd expire-time
                        if(timers_.top() == timer) {
                            timerfdSet(timerfd_, when);
                        }
                    });
    return timer;
}

void TimerQueue::cancelTimer(Timer::sptr timer) {
    loop_->runInLoop([timer] {
                         timer->cancel();
                         // delay deletion
                         timer->setTimerCallback(nullptr);
                     });
}

void TimerQueue::handleRead() {
    loop_->assertInLoopThread();
    timerfdRead(timerfd_);
    
    Timestamp now(clock::now());

    while (!timers_.empty()) {
        auto timer = timers_.top();
        if (!timer->expired(now)) {
            break;
        }

        if (!timer->canceled()) {
            timer->run();
        }

        timers_.pop();

        if (!timer->canceled() && timer->repeat()) {
            timer->restart();
            timers_.emplace(std::move(timer));
        }
        // true delete
    }
    // update timerfd expire-time
    if (!timers_.empty())
        timerfdSet(timerfd_, timers_.top()->when());
}
