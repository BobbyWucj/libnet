/*
 * TcpServerSingle.h
 * Created on 2023/3/9
 * Copyright (c) 2023 BobbyWucj
 *
 * Sub Reactor
 */

#ifndef LIBNET_TCPSERVERSINGLE_H
#define LIBNET_TCPSERVERSINGLE_H

#include "core/TcpReactor.h"

namespace libnet 
{

class EventLoop;

class TcpSubReactor : public TcpReactor 
{
public:
    using ptr = std::unique_ptr<TcpSubReactor>;

    TcpSubReactor(EventLoop* loop, const InetAddress& local, const Nanoseconds heartbeat);
    ~TcpSubReactor() = default;

    void start() override;

private:
    void newConnection(int connfd, const InetAddress& local, const InetAddress& peer) override;
    void closeConnection(const TcpConnectionPtr& conn) override;
    void closeConnectionInLoop(const TcpConnectionPtr &connPtr) override;
};

} // namespace libnet

#endif // LIBNET_TCPSERVERSINGLE_H
