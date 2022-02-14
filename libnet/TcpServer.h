#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "Callbacks.h"
#include "InetAddress.h"
#include "noncopyable.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

namespace libnet
{

class EventLoop;
class TcpServerSingle;
class EventLoopThread;

class TcpServer : noncopyable
{
public:
    TcpServer(EventLoop* loop, const InetAddress& local);
    ~TcpServer();
    
    void setNumThreads(size_t numThreads);

    void start();

    void setThreadInitCallback(const ThreadInitCallback &threadInitCallback) { threadInitCallback_ = threadInitCallback; }
    void setConnectionCallback(const ConnectionCallback &connectionCallback) { connectionCallback_ = connectionCallback; }
    void setMessageCallback(const MessageCallback &messageCallback) { messageCallback_ = messageCallback; }
    void setWriteCompleteCallback(const WriteCompleteCallback &writeCompleteCallback) { writeCompleteCallback_ = writeCompleteCallback; }

private:
    void startInLoop();
    void runInThread(const size_t index);

    using ThreadPtr = std::unique_ptr<std::thread>;
    using ThreadPtrList = std::vector<ThreadPtr>;
    using TcpServerSinglePtr = std::unique_ptr<TcpServerSingle>;
    using EventLoopList = std::vector<EventLoop*>;

    EventLoop*                      baseLoop_;
    TcpServerSinglePtr              baseServer_;
    ThreadPtrList                   threads_;
    EventLoopList                   eventLoops_;
    size_t                          numThreads_;
    std::atomic<bool>               started_;
    InetAddress                     local_;
    mutable std::mutex              mutex_;
    mutable std::condition_variable cond_;

    ThreadInitCallback              threadInitCallback_;
    ConnectionCallback              connectionCallback_;
    MessageCallback                 messageCallback_;
    WriteCompleteCallback           writeCompleteCallback_;
};


}

#endif // TCPSERVER_H
