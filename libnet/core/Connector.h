#ifndef LIBNET_CONNECTOR_H
#define LIBNET_CONNECTOR_H

#include "core/Callbacks.h"
#include "core/Channel.h"
#include "core/InetAddress.h"
#include "utils/noncopyable.h"

namespace libnet {

class EventLoop;

class Connector : noncopyable
{
public:
    Connector(EventLoop* loop, const InetAddress& peer);
    ~Connector();

    void start();

    void setNewConnectionCallback(
        const NewConnectionCallback& newConnectionCallback) {
        newConnectionCallback_ = newConnectionCallback;
    }

    void setErrorCallback(const ErrorCallback& errorCallback) {
        errorCallback_ = errorCallback;
    }

private:
    void handleWrite();

    EventLoop*            loop_;
    const InetAddress     peer_;
    const int             cfd_;
    bool                  connected_;
    bool                  started_;
    Channel               channel_;
    NewConnectionCallback newConnectionCallback_;
    ErrorCallback         errorCallback_;
};

}  // namespace libnet

#endif  // LIBNET_CONNECTOR_H
