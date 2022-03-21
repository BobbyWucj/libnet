#ifndef LIBNET_TCPCONNECTION_H
#define LIBNET_TCPCONNECTION_H


#include "Callbacks.h"
#include "Channel.h"
#include "EventLoop.h"
#include "libnet/Timestamp.h"
#include "libnet/base/noncopyable.h"
#include "InetAddress.h"
#include "Buffer.h"

#include <atomic>
#include <cstddef>
#include <string>
#include <any>
#include <utility>

namespace libnet
{

class EventLoop;

class TcpConnection : private noncopyable, 
                      public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop* loop,
                  int cfd,
                  const InetAddress& local,
                  const InetAddress& peer,
                  const Nanoseconds heartbeat);
    ~TcpConnection();

    void connectionEstablished();
    bool connected() const { return state_ == kConnected; }
    bool disconnected() const { return state_ == kDisconnected; }

    void send(const std::string& data);
    void send(const char* data, size_t len);
    void send(Buffer& buffer);
    void shutdown();
    void forceClose();

    void startRead();
    void stopRead();
    // not thread safe
    bool isReading() { return channel_->isReading(); }

    void setMessageCallback(const MessageCallback &messageCallback) { messageCallback_ = messageCallback; }
    void setCloseCallback(const CloseCallback &closeCallback) { closeCallback_ = closeCallback; }
    void setWriteCompleteCallback(const WriteCompleteCallback &writeCompleteCallback) { writeCompleteCallback_ = writeCompleteCallback; }
    void setHighWaterMarkCallback(const HighWaterMarkCallback &highWaterMarkCallback, const size_t highWaterMark) { 
        highWaterMarkCallback_ = highWaterMarkCallback;
        highWaterMark_ = highWaterMark;
    }
    void setConnectionCallback(const ConnectionCallback &connectionCallback) { connectionCallback_ = connectionCallback; }

    void setMessageCallback(MessageCallback &&messageCallback) { messageCallback_ = std::move(messageCallback); }
    void setCloseCallback(CloseCallback &&closeCallback) { closeCallback_ = std::move(closeCallback); }
    void setWriteCompleteCallback(WriteCompleteCallback &&writeCompleteCallback) { writeCompleteCallback_ = std::move(writeCompleteCallback); }
    void setHighWaterMarkCallback(HighWaterMarkCallback &&highWaterMarkCallback, const size_t highWaterMark) { 
        highWaterMarkCallback_ = std::move(highWaterMarkCallback);
        highWaterMark_ = highWaterMark;
    }
    void setConnectionCallback(ConnectionCallback &&connectionCallback) { connectionCallback_ = std::move(connectionCallback); }

    const InetAddress& local() const { return *local_; }
    const InetAddress& peer() const { return *peer_; }
    std::string name() const { return peer_->toIpPort() + " -> " + local_->toIpPort(); }

    const Buffer& inputBuffer() const { return *inputBuffer_; }
    const Buffer& outputBuffer() const { return *outputBuffer_; }

    const std::any& getContext() const { return context_; }
    std::any* getMutableContext() { return &context_; }
    void setContext(const std::any& context) { context_ = context; }

    Timer* timer() const { return timer_; }
    void setTimer(Timer* timer) { timer_ = timer; }

    void onInactiveConn();

    EventLoop* getLoop() const { return loop_; }

private:
    enum State {
        kConnecting,
        kConnected,
        kDisconnecting,
        kDisconnected
    };

    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const std::string& message);
    void sendInLoop(const char* data, size_t len);

    void shutdownInLoop();
    void forceCloseInLoop();

    EventLoop*                      loop_;
    std::atomic<int>                state_;
    int                             cfd_;
    std::unique_ptr<Channel>        channel_;
    std::unique_ptr<InetAddress>    local_;
    std::unique_ptr<InetAddress>    peer_;
    std::unique_ptr<Buffer>         inputBuffer_;
    std::unique_ptr<Buffer>         outputBuffer_;
    std::any                        context_;
    Timer*                          timer_;
    Nanoseconds                     heartbeat_;

    MessageCallback                 messageCallback_;
    CloseCallback                   closeCallback_;
    WriteCompleteCallback           writeCompleteCallback_;
    HighWaterMarkCallback           highWaterMarkCallback_;
    ConnectionCallback              connectionCallback_;
    size_t                          highWaterMark_;

};

}

#endif // LIBNET_TCPCONNECTION_H
