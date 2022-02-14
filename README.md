# LIBNET

## LIBNET 是采用 Reactor 模式的 one loop per thread 多线程网络库

相关实现参考 muduo，有以下亮点：

- 多路复用 I/O 仅支持 epoll, 不支持 poll 和 select。

- 多线程使用 std::thread, 不是直接使用/封装 POSIX API。

- 同步互斥原语使用 C++ 标准库，不是直接使用/封装 POSIX API。

- 定时器依赖于 std::chrono, 不是自己实现 TimeStamp 类。

- 默认为 listen socket 开启 `SO_REUSEPORT` 选项：

  SO_REUSEPORT支持多个进程或者线程绑定到同一端口，提高服务器程序的性能，可以实现：

  - 允许多个套接字 bind()/listen() 同一个TCP/UDP端口
    - 每一个线程拥有自己的服务器套接字
    - 在服务器套接字上没有了锁的竞争
  - 内核层面实现负载均衡

  有了SO_RESUEPORT后，每个 进程/线程 可以自己创建socket、bind、listen、accept相同的地址和端口，各自是独立平等的。让 多进程/线程 监听同一个端口，各个进程中`accept socketfd`不一样，有新连接建立时，内核只会唤醒一个进程/线程来`accept`，并且**保证唤醒的均衡性。**

- 具有简单的日志输出功能，便于调试。

## Example

示例见 ./example

目前实现有：

- echo_server, echo_client
- ...

## Install

```bash
$ git clone git@github.com:BobbyWucj/libnet.git
$ cd libnet
$ ./build.sh 
$ ./build.sh install
```

安装目录默认为 ../libnet-build/

## References

- [Muduo is a multithreaded C++ network library based on the reactor pattern.](https://github.com/chenshuo/muduo)