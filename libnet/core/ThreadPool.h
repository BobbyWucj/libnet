#ifndef LIBNET_THREADPOOL_H
#define LIBNET_THREADPOOL_H

#include "core/Callbacks.h"
#include "utils/noncopyable.h"
#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>


namespace libnet
{

class ThreadPool : noncopyable
{
public:
    explicit ThreadPool(size_t numThread,
                        size_t maxQueueSize = 65536,
                        const ThreadInitCallback& cb = nullptr);
    ~ThreadPool();

    void runTask(const Task& task);
    void runTask(Task&& task);

    void stop();
    size_t numThreads() const
    { return threads_.size(); }

private:
    void runInThread(size_t index);
    Task take();

    using ThreadPtr = std::unique_ptr<std::thread>;
    using ThreadList = std::vector<ThreadPtr>;

    ThreadList threads_;
    std::mutex mutex_;
    std::condition_variable notFull_;
    std::condition_variable notEmpty_;
    std::deque<Task> taskQueue_;
    const size_t maxQueueSize_;
    std::atomic_bool running_;
    ThreadInitCallback threadInitCallback_;

};

}

#endif // LIBNET_THREADPOOL_H
