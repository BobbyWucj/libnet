#include <cassert>
#include "EventLoop.h"
#include "Channel.h"

using namespace libnet;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(0),
      revents_(0),
      tied_(false),
      handlingEvents_(false),
      polling_(false),
      readCallback_(nullptr),
      writeCallback_(nullptr),
      closeCallback_(nullptr),
      errorCallback_(nullptr)
{}

Channel::~Channel() {
    assert(!handlingEvents_);
}

void Channel::handleEvents() {
    loop_->assertInLoopThread();

    if(tied_) {
        auto guard = tie_.lock();
        if(guard) {
            handleEventsWithGuard();
        }
    } else {
        handleEventsWithGuard();
    }
}

void Channel::handleEventsWithGuard() {
    handlingEvents_ = true;
    if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
        if(closeCallback_)
            closeCallback_();
    }
    if(revents_ & EPOLLERR) {
        if(errorCallback_)
            errorCallback_();
    }
    if(revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if(readCallback_)
            readCallback_();
    }
    if(revents_ & EPOLLOUT) {
        if(writeCallback_)
            writeCallback_();
    }
    handlingEvents_ = false;
}

void Channel::update() { 
    loop_->updateChannel(this);
}

void Channel::remove() { 
    assert(polling());
    loop_->removeChannel(this);
}



