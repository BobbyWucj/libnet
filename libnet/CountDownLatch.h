#ifndef COUNTDOWNLATCH_H
#define COUNTDOWNLATCH_H

#include <mutex>
#include <condition_variable>
#include "noncopyable.h"

namespace libnet {

class CountDownLatch : noncopyable
{
public:
    explicit CountDownLatch(int count)
        : count_(count) {}
    void wait() {
        std::unique_lock<std::mutex> lock(mutex_);
        while(count_ > 0) {
            cond_.wait(lock);
        }
    }
    void countDown() {
        std::lock_guard<std::mutex> guard(mutex_);
        --count_;
        if(count_ <= 0) {
            cond_.notify_all();
        }
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable cond_;
    int count_;
};

}; 

#endif // COUNTDOWNLATCH_H
