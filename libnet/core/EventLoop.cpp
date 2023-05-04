#include <cassert>
#include <memory>
#include <numeric>
#include <signal.h>
#include <sys/eventfd.h>
#include <sys/types.h>
#include <syscall.h>
#include <thread>
#include <unistd.h>
#include <utility>

#include "core/EPoller.h"
#include "core/EventLoop.h"
#include "core/Timestamp.h"
#include "logger/Logger.h"

using namespace libnet;

namespace {

__thread EventLoop* t_loopInThisThread = nullptr;

class IgnoreSigPipe
{
public:
    IgnoreSigPipe() { ::signal(SIGPIPE, SIG_IGN); }
};

IgnoreSigPipe ignoreSigPipe;

}  // anonymous namespace

EventLoop::EventLoop()
    : tid_(std::this_thread::get_id()),
      quit_(false),
      poller_(std::make_unique<EPoller>(this)),
      timerQueue_(this),
      doingPendingTasks_(false),
      wakeupFd_(::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)),
      wakeupChannel_(std::make_unique<Channel>(this, wakeupFd_)) {
    // FIXME : LOG tid
    LOG_INFO << "EventLoop createt " << this << " in thread ";
    if (wakeupFd_ <= 0) {
        LOG_FATAL << "EventLoop::eventfd() fail to create";
    }
    wakeupChannel_->setReadCallback([this] { handleRead(); });
    wakeupChannel_->enableReading();
    if (t_loopInThisThread) {
        LOG_FATAL << "Another EventLoop " << t_loopInThisThread
                  << " exits in this thread";
    }
    else {
        t_loopInThisThread = this;
    }
}

EventLoop::~EventLoop() {
    assert(t_loopInThisThread == this);
    t_loopInThisThread = nullptr;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread() {
    return t_loopInThisThread;
}

void EventLoop::loop() {
    assertInLoopThread();
    LOG_TRACE << "EventLoop " << this << " polling";
    quit_ = false;
    while (!quit_) {
        activeChannels_.clear();

        poller_->poll(activeChannels_);

        for (auto& channel : activeChannels_) {
            channel->handleEvents();
        }
        doPendingTasks();
    }
    LOG_TRACE << "EventLoop " << this << " stop looping";
}

void EventLoop::quit() {
    assert(!quit_);
    quit_ = true;
    if (!isInLoopThread())
        wakeup();
}

void EventLoop::updateChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    poller_->updateChannel(channel);
    assertInLoopThread();
    LOG_TRACE << "EventLoop " << this << " update Channel " << channel;
}

void EventLoop::removeChannel(Channel* channel) {
    assertInLoopThread();
    channel->disableAll();
    LOG_TRACE << "EventLoop " << this << " remove Channel " << channel;
}

Timer::sptr EventLoop::runAt(Timestamp when, TimerCallback callback) {
    return timerQueue_.addTimer(std::move(callback), when);
}

Timer::sptr EventLoop::runAfter(Nanoseconds interval, TimerCallback callback) {
    return timerQueue_.addTimer(std::move(callback), clock::now() + interval,
                                interval, false);
}

Timer::sptr EventLoop::runEvery(Nanoseconds interval, TimerCallback callback) {
    return timerQueue_.addTimer(std::move(callback), clock::now() + interval,
                                interval, true);
}

void EventLoop::cancelTimer(Timer::sptr timer) {
    timerQueue_.cancelTimer(timer);
}

void EventLoop::runInLoop(const Task& task) {
    if (isInLoopThread()) {
        task();
    }
    else {
        queueInLoop(task);
    }
}

void EventLoop::runInLoop(Task&& task) {
    if (isInLoopThread()) {
        task();
    }
    else {
        queueInLoop(std::move(task));
    }
}

void EventLoop::queueInLoop(const Task& task) {
    {
        std::lock_guard<std::mutex> guard(mutex_);
        pendingTasks_.push_back(task);
    }
    if (!isInLoopThread() || doingPendingTasks_)
        wakeup();
}

void EventLoop::queueInLoop(Task&& task) {
    {
        std::lock_guard<std::mutex> guard(mutex_);
        pendingTasks_.push_back(std::move(task));
    }
    if (!isInLoopThread() || doingPendingTasks_)
        wakeup();
}

void EventLoop::doPendingTasks() {
    assertInLoopThread();
    std::vector<Task> tasks;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        tasks.swap(pendingTasks_);
    }
    doingPendingTasks_ = true;
    for (Task& task : tasks) {
        task();
    }
    doingPendingTasks_ = false;
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
        LOG_SYSERR << "EventLoop::wakeup() should ::write " << sizeof(one)
                   << " bytes";
}

void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
        LOG_SYSERR << "EventLoop::wakeup() should ::read " << sizeof(one)
                   << " bytes";
}

bool EventLoop::isInLoopThread() const {
    return tid_ == std::this_thread::get_id();
}
