#ifndef LIBNET_ACCEPTOR_H
#define LIBNET_ACCEPTOR_H

#include "core/Callbacks.h"
#include "utils/noncopyable.h"
#include "core/InetAddress.h"
#include "core/Channel.h"
#include <algorithm>
#include <memory>

namespace libnet
{

class EventLoop;

class Acceptor : noncopyable
{
public:
    using ptr = std::unique_ptr<Acceptor>;

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
