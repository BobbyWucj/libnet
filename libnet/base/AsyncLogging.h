#ifndef LIBNET_BASE_ASYNCLOGGING_H
#define LIBNET_BASE_ASYNCLOGGING_H

#include <cstddef>
#include <functional>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "CountDownLatch.h"
#include "LogStream.h"
#include "noncopyable.h"

namespace libnet {
namespace logger {

class AsyncLogging : noncopyable 
{
public:
    explicit AsyncLogging(const std::string basename, int flushInterval = 2);
    ~AsyncLogging() {
        if (running_) 
            stop();
    }
    void append(const char* logline, size_t len);

    void start() {
        running_ = true;
        thread_ = std::thread(
            [this]{ this->threadFunc(); }
        );
        latch_.wait();
    }

    void stop() {
        running_ = false;
        cond_.notify_one();
        thread_.join();
    }

private:
    void threadFunc();

    using Buffer = FixedBuffer<kLargeBuffer>;
    using BufferPtr = std::shared_ptr<Buffer>;
    using BufferVector = std::vector<BufferPtr>;

    const int flushInterval_;
    bool running_;
    std::string basename_;

    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;

    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BufferVector buffers_;

    CountDownLatch latch_;
};

} // namespace logger
} // namespace libnet

#endif // LIBNET_BASE_ASYNCLOGGING_H
