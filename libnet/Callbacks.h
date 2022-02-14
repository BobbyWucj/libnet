#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <functional>
#include <memory>

namespace libnet {
using namespace std::string_literals;

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;
using std::placeholders::_5;

class Buffer;
class TcpConnection;
class InetAddress;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>; // 消息完全发送完毕的回调函数
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr &, size_t)>; // 消息阻塞发不出积累高水位时的回调函数
using MessageCallback = std::function<void(const TcpConnectionPtr &, Buffer &)>;
using ErrorCallback = std::function<void()>;
using NewConnectionCallback = std::function<void(int cfd, const InetAddress &local, const InetAddress &peer)>;

using Task = std::function<void()>;
using TimerCallback = std::function<void()>;
using ThreadInitCallback = std::function<void(size_t index)>;

void defaultThreadInitCallback(size_t index);
void defaultConnectionCallback(const TcpConnectionPtr &conn);
void defaultMessageCallback(const TcpConnectionPtr &conn, Buffer &buffer);

} // namespace libnet

#endif // CALLBACKS_H
