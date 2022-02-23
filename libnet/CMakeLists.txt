
SET(SRC
    Acceptor.cpp
    Buffer.cpp
    Channel.cpp
    Connector.cpp
    EPoller.cpp
    EventLoop.cpp
    EventLoopThread.cpp
    InetAddress.cpp
    TcpClient.cpp
    TcpConnection.cpp
    TcpServer.cpp
    TcpServerSingle.cpp
    TimerQueue.cpp)

add_library(libnet STATIC ${SRC})
target_link_libraries(libnet pthread)
target_link_libraries(libnet logger)

SET(HEADERS
    Acceptor.h
    Buffer.h
    Callbacks.h
    Channel.h
    Connector.h
    EPoller.h
    EventLoop.h
    EventLoopThread.h
    InetAddress.h
    TcpClient.h
    TcpConnection.h
    TcpServer.h
    TcpServerSingle.h
    Timer.h
    TimerQueue.h
    Timestamp.h)

install(TARGETS libnet DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include)

add_subdirectory(base)