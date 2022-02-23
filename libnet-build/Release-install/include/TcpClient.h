#ifndef LIBNET_TCPCLIENT_H
#define LIBNET_TCPCLIENT_H

#include "Callbacks.h"
#include "Channel.h"
#include "InetAddress.h"
#include "Timer.h"
#include "libnet/base/noncopyable.h"
#include "Connector.h"
#include <memory>

namespace libnet
{

class TcpClient : noncopyable
{
public:
    TcpClient(EventLoop* loop, const InetAddress& peer);
    ~TcpClient();

    void start();

    void setConnectionCallback(const ConnectionCallback &connectionCallback) 
    { connectionCallback_ = connectionCallback; }

    void setMessageCallback(const MessageCallback &messageCallback) 
    { messageCallback_ = messageCallback; }

    void setWriteCompleteCallback(const WriteCompleteCallback &writeCompleteCallback) 
    { writeCompleteCallback_ = writeCompleteCallback; }

    void setErrorCallback(const ErrorCallback& errorCallback)
    { connector_->setErrorCallback(errorCallback); }

private:
    void retry();
    void newConnection(int connfd, const InetAddress& local, const InetAddress& peer);
    void closeConnection(const TcpConnectionPtr& conn);

    using ConnectorPtr = std::unique_ptr<Connector>;

    EventLoop*              loop_;
    bool                    connected_;
    InetAddress             peer_;
    Timer*                  retryTimer_;
    ConnectorPtr            connector_;
    TcpConnectionPtr        connection_;
    ConnectionCallback      connectionCallback_;
    MessageCallback         messageCallback_;
    WriteCompleteCallback   writeCompleteCallback_;
};

}

#endif // LIBNET_TCPCLIENT_H
