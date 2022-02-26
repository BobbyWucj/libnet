# WebServer

## WebServer 是一个基于 (Libnet)[https://github.com/BobbyWucj/libnet] 的 Http 服务器

- 底层socket网络编程基于 Libnet 实现；此项目主要专注于 Http 协议的解析与处理

- 使用状态机解析 Http 请求报文，支持解析 `GET`/`HEAD` 请求，并返回用户请求的资源

- 使用 `mmap()` 为用户请求的文件资源创建共享内存映射区，高效地返回用户请求的资源

- 使用基于小根堆的定时器处理非活动连接