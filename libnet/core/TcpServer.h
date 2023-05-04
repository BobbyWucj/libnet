#ifndef LIBNET_TCPSERVER_H
#define LIBNET_TCPSERVER_H

#include "core/Callbacks.h"
#include "core/EventLoopThreadPool.h"
#include "core/InetAddress.h"
#include "core/TcpReactor.h"
#include "core/Timestamp.h"
#include "utils/noncopyable.h"

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace libnet {

class EventLoop;
class EventLoopThread;

class TcpServer : noncopyable
{
public:
    TcpServer(EventLoop*         loop,
              const InetAddress& local,
              bool               reusePort = true,
              const Nanoseconds  heartbeat = 5s);
    ~TcpServer();

    // should be called before start
    void disableReusePort() { reusePort_ = false; }
    void setNumThreads(size_t numThreads);
    void start();

    void setThreadInitCallback(const ThreadInitCallback& threadInitCallback) {
        threadInitCallback_ = threadInitCallback;
    }
    void setConnectionCallback(const ConnectionCallback& connectionCallback) {
        connectionCallback_ = connectionCallback;
    }
    void setMessageCallback(const MessageCallback& messageCallback) {
        messageCallback_ = messageCallback;
    }
    void setWriteCompleteCallback(
        const WriteCompleteCallback& writeCompleteCallback) {
        writeCompleteCallback_ = writeCompleteCallback;
    }

    size_t             numThreads() const { return numThreads_; }
    EventLoop*         getLoop() const { return baseLoop_; }
    const std::string& ipPort() const { return ipPort_; }

private:
    void startInLoop();
    void runInThread(const size_t index);
    void newConnection(int                connfd,
                       const InetAddress& local,
                       const InetAddress& peer);

    EventLoop*      baseLoop_;
    TcpReactor::ptr reactor_;

    size_t            numThreads_;
    std::atomic_bool  started_;
    InetAddress       local_;
    const std::string ipPort_;

    Nanoseconds heartbeat_;

    bool reusePort_;

    ThreadInitCallback    threadInitCallback_;
    ConnectionCallback    connectionCallback_;
    MessageCallback       messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
};

}  // namespace libnet

#endif  // LIBNET_TCPSERVER_H
