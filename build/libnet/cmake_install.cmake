# Install script for directory: /home/bobbywu/Desktop/Developer/libnet/libnet

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/bobbywu/Desktop/Developer/libnet/build/lib/liblibnet.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/home/bobbywu/Desktop/Developer/libnet/libnet/Acceptor.cpp"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/Acceptor.h"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/Buffer.cpp"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/Buffer.h"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/CMakeLists.txt"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/Callbacks.h"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/Channel.cpp"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/Channel.h"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/Connector.cpp"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/Connector.h"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/CountDownLatch.h"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/EPoller.cpp"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/EPoller.h"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/EventLoop.cpp"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/EventLoop.h"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/EventLoopThread.cpp"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/EventLoopThread.h"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/InetAddress.cpp"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/InetAddress.h"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/Logger.c"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/Logger.h"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/TcpClient.cpp"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/TcpClient.h"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/TcpConnection.cpp"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/TcpConnection.h"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/TcpServer.cpp"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/TcpServer.h"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/TcpServerSingle.cpp"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/TcpServerSingle.h"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/Timer.h"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/TimerQueue.cpp"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/TimerQueue.h"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/Timestamp.h"
    "/home/bobbywu/Desktop/Developer/libnet/libnet/noncopyable.h"
    )
endif()

