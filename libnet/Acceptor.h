#ifndef LIBNET_ACCEPTOR_H
#define LIBNET_ACCEPTOR_H

#include "Callbacks.h"
#include "libnet/base/noncopyable.h"
#include "InetAddress.h"
#include "Channel.h"

namespace libnet
{

class EventLoop;

class Acceptor : noncopyable
{
public:
    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reusePort = true);
    ~Acceptor();

    void listen();

    bool listening() const { return listening_; }

    void setNewConnectionCallback(const NewConnectionCallback &newConnectionCallback) 
    { newConnectionCallback_ = newConnectionCallback; }

private:
    void handleRead();

    bool                        listening_;
    int                         listenFd_;
    int                         emfileFd_;
    EventLoop*                  loop_;
    std::unique_ptr<Channel>    listenChannel_;
    InetAddress                 listenAddr_;
    NewConnectionCallback       newConnectionCallback_;
    bool                        reusePort_;
};


} // namespace libnet


#endif // LIBNET_ACCEPTOR_H
