#ifndef LIBNET_TIMER_H
#define LIBNET_TIMER_H

#include <any>
#include <cassert>
#include "Callbacks.h"
#include "Channel.h"
#include "Timestamp.h"

namespace libnet
{

class Timer: noncopyable
{
public:
    Timer(TimerCallback callback, Timestamp when, Nanoseconds interval, bool repeat)
        : callback_(std::move(callback)),
        when_(when),
        interval_(interval),
        repeat_(repeat),
        canceled_(false)
    {}

    void run() {
        if(callback_)
            callback_();    
    }

    void restart() {
        assert(repeat_);
        when_ += interval_;
    }

    void cancel() {
        assert(!canceled_);
        canceled_ = true;
    }

    bool expired(Timestamp now) const { return now >= when_; }
    
    // getter
    Timestamp when() const { return when_; }
    void setWhen(const Timestamp when) { when_ = when; }
    bool repeat() const { return repeat_; }
    bool canceled() const { return canceled_; }

    const TimerCallback& timerCallback() const { return callback_; }
    void setTimerCallback(TimerCallback callback) { callback_ = std::move(callback); }

    Nanoseconds interval() const { return interval_; }
    void setInterval(const Nanoseconds &interval) { interval_ = interval; }

private:
    TimerCallback callback_;
    Timestamp when_;
    Nanoseconds interval_;
    const bool repeat_;
    bool canceled_;
};

} // namespace libnet

#endif // LIBNET_TIMER_H
