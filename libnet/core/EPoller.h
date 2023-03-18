#ifndef LIBNET_EPOLLER_H
#define LIBNET_EPOLLER_H

#include <vector>
#include "utils/noncopyable.h"

namespace libnet
{

class EventLoop;
class Channel;

class EPoller : noncopyable
{
public:
    using ChannelList = std::vector<Channel*>;
    using EventList = std::vector<struct epoll_event>;

    explicit EPoller(EventLoop* loop);
    ~EPoller();

    void poll(ChannelList& activeChannels, int timeout = -1);
    void updateChannel(Channel* channel);

private:
    void updateChannel(int op, Channel* channel);

    EventLoop*  loop_;
    EventList   events_;
    int         epollfd_;
};


} // namespace of libnet

#endif // LIBNET_EPOLLER_H
