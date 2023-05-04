#include "core/TcpClient.h"
#include "core/TcpConnection.h"

using namespace libnet;

class EchoClient : libnet::noncopyable
{
public:
    EchoClient(EventLoop* loop, const InetAddress& peer);

    void start();

    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer& buffer);
    void getLineAndSend();

private:
    TcpConnectionPtr conn_;
    EventLoop*       loop_;
    TcpClient        client_;
};