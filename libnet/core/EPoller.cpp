#include <unistd.h>
#include <sys/epoll.h>
#include <cassert>

#include "logger/Logger.h"
#include "core/EventLoop.h"
#include "core/EPoller.h"

using namespace libnet;

EPoller::EPoller(EventLoop* loop)
    : loop_(loop),
      events_(128),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC))
{
    if(epollfd_ == -1) {
        LOG_SYSFATAL << "Epoller::epoll_create1()";
    }
}

EPoller::~EPoller() {
    ::close(epollfd_);
}

// return activeChannels to EventLoop::loop
void EPoller::poll(ChannelList& activeChannels, int timeout) {
    loop_->assertInLoopThread();
    int max_events = static_cast<int>(events_.size());
    int num_events = epoll_wait(epollfd_, events_.data(), max_events, timeout);
    if (num_events == -1){
        if (errno != EINTR)
            LOG_SYSERR << "EPoller::poll()";
    } else if (num_events > 0) {
        LOG_TRACE << num_events << " events happend";
        for (int i = 0; i < num_events; ++i) {
            Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
            channel->setRevents(events_[i].events);
            activeChannels.push_back(channel);
        }
        if(num_events == max_events) {
            events_.resize(2 * events_.size());
        }
    } else if(num_events == 0) {
        LOG_TRACE << "nothing happended";
    }
}

// onConnection or onDisconnection or onModify
void EPoller::updateChannel(Channel* channel) {
    loop_->assertInLoopThread();
    int op = 0;
    if (!channel->polling()) {
        assert(!channel->isNoneEvents());
        op = EPOLL_CTL_ADD;
        channel->setPolling(true);
    } else if (!channel->isNoneEvents()) {
        op = EPOLL_CTL_MOD;
    } else {
        op = EPOLL_CTL_DEL;
        channel->setPolling(false);
    }
    updateChannel(op, channel);
}

void EPoller::updateChannel(int op, Channel* channel) {
    struct epoll_event event;
    event.events = channel->events();
    event.data.ptr = channel;
    int ret = ::epoll_ctl(epollfd_, op, channel->fd(), &event);
    if(ret == -1)
        LOG_SYSERR << "EPoller::updateChannel(op, channel)";
}