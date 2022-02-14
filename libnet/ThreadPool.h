#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "Callbacks.h"
#include "noncopyable.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <deque>

namespace libnet
{

class ThreadPool : noncopyable
{
public:
    explicit ThreadPool(size_t numThreads,
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

    ThreadList              threads_;
    std::mutex              mutex_;
    std::condition_variable notEmpty_;
    std::condition_variable notFull_;
    std::deque<Task>        taskQueue_;
    const size_t            maxQueueSize_;
    std::atomic_bool        running_;
    ThreadInitCallback      threadInitCallback_;
};

} // namespace libnet

#endif // THREADPOOL_H
