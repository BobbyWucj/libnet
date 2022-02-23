#ifndef LIBNET_CHANNEL_H
#define LIBNET_CHANNEL_H

#include <functional>
#include <memory>
#include <sys/epoll.h>

#include "libnet/base/noncopyable.h"

namespace libnet
{

class EventLoop;

class Channel : noncopyable
{
public:
    using EventCallback = std::function<void()>;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    void handleEvents();

    void setReadCallback(const EventCallback &readCallback) { readCallback_ = readCallback; }
    void setWriteCallback(const EventCallback &writeCallback) { writeCallback_ = writeCallback; }
    void setCloseCallback(const EventCallback &closeCallback) { closeCallback_ = closeCallback; }
    void setErrorCallback(const EventCallback &errorCallback) { errorCallback_ = errorCallback; }

    int fd() const { return fd_; }

    int events() const { return events_; }
    void setRevents(int revents) { revents_ = revents; }

    bool polling() const { return polling_; }
    void setPolling(bool polling) { polling_ = polling; }

    void enableReading() { events_ |= kReadEvent; update(); }
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableReading() { events_ &= ~kReadEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update(); }

    bool isNoneEvents() const { return events_ == 0; }
    bool isReading() const { return events_ & EPOLLIN; }
    bool isWriting() const { return events_ & EPOLLOUT; }

    void tie(const std::shared_ptr<void>& obj) {
        tie_ = obj;
        tied_ = true;
    }

    EventLoop* ownerLoop() {
        return loop_;
    }

    void remove();

private:
    void update();
    void handleEventsWithGuard();

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;
    const int fd_;
    int events_;
    int revents_;
    std::weak_ptr<void> tie_; // 延长TcpConnection的生命周期
    bool tied_;
    bool handlingEvents_;
    bool polling_;
    
    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;

};

} // namespace of libnet

#endif // LIBNET_CHANNEL_H
