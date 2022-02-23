#ifndef LIBNET_TIMER_H
#define LIBNET_TIMER_H

#include <cassert>
#include "Callbacks.h"
#include "Channel.h"
#include "Timestamp.h"

namespace libnet
{

class Timer: noncopyable
{
public:
    Timer(TimerCallback callback, Timestamp when, Nanoseconds interval)
        : callback_(std::move(callback)),
        when_(when),
        interval_(interval),
        repeat_(interval_ > Nanoseconds::zero()),
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
    bool repeat() const { return repeat_; }
    bool canceled() const { return canceled_; }

private:
    TimerCallback callback_;
    Timestamp when_;
    const Nanoseconds interval_;
    const bool repeat_;
    bool canceled_;
};

} // namespace libnet

#endif // LIBNET_TIMER_H
