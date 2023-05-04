CMAKE_MINIMUM_REQUIRED(VERSION 3.14)    # define the minimum required version of CMake to be used
SET(BUILD_SHARED_LIBS OFF)              # We expect external libraries to be linked statically.
SET(CMAKE_CXX_STANDARD 17)              # Compile as C++17. GoogleTest requires at least C++14
SET(CMAKE_CXX_STANDARD_REQUIRED ON)     # Require C++17 support.

# Define the project meta information
PROJECT(libnet
        VERSION 2023.02
        DESCRIPTION "A C++ High Performance Network Web Server Framework"
        LANGUAGES C CXX
        )

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/lib)

### Include

# Includes header file path
SET(LIBNET_SRC_DIR ${PROJECT_SOURCE_DIR}/libnet)
SET(LIBNET_ECHO_DIR ${PROJECT_SOURCE_DIR}/example/Echo)
SET(LIBNET_WEBSERVER_DIR ${PROJECT_SOURCE_DIR}/example/WebServer)
SET(LIBNET_TEST_DIR ${PROJECT_SOURCE_DIR}/test)

### Build
SET(CMAKE_COMPILER_FLAG -O3 -Wall -Wextra -pedantic)

FILE(GLOB LIBNET_UTILS_HEADER RELATIVE ${CMAKE_SOURCE_DIR} "libnet/utils/*.h")
install(FILES ${LIBNET_UTILS_HEADER} DESTINATION include/utils)

# Build the libnet core library
FILE(GLOB LIBNET_CORE_HEADER RELATIVE ${CMAKE_SOURCE_DIR} "libnet/core/*.h")
FILE(GLOB LIBNET_CORE_SOURCE RELATIVE ${CMAKE_SOURCE_DIR} "libnet/core/*.cpp")

ADD_LIBRARY(libnet ${LIBNET_CORE_SOURCE})

TARGET_LINK_LIBRARIES(libnet pthread)
TARGET_COMPILE_OPTIONS(libnet PRIVATE ${CMAKE_COMPILER_FLAG})
TARGET_INCLUDE_DIRECTORIES(
        libnet
        PUBLIC ${LIBNET_SRC_DIR}
)

# install(TARGETS libnet DESTINATION lib)
install(FILES ${LIBNET_CORE_HEADER} DESTINATION include/core)

# Build the libnet logger library
FILE(GLOB LIBNET_LOGGER_HEADER RELATIVE ${CMAKE_SOURCE_DIR} "libnet/logger/*.h")
FILE(GLOB LIBNET_LOGGER_SOURCE RELATIVE ${CMAKE_SOURCE_DIR} "libnet/logger/*.cpp")

ADD_LIBRARY(logger ${LIBNET_LOGGER_SOURCE})

TARGET_COMPILE_OPTIONS(logger PRIVATE ${CMAKE_COMPILER_FLAG})
TARGET_INCLUDE_DIRECTORIES(
        logger
        PUBLIC ${LIBNET_SRC_DIR}
)

# install(TARGETS logger DESTINATION lib)
install(FILES ${LIBNET_LOGGER_HEADER} DESTINATION include/logger)

# Build the echo server
ADD_EXECUTABLE(echo_server ${LIBNET_ECHO_DIR}/EchoServer.cpp)
TARGET_LINK_LIBRARIES(echo_server libnet)
TARGET_LINK_LIBRARIES(echo_server logger)

TARGET_COMPILE_OPTIONS(echo_server PRIVATE ${CMAKE_COMPILER_FLAG})
TARGET_INCLUDE_DIRECTORIES(
        echo_server
        PUBLIC ${LIBNET_SRC_DIR}
        PUBLIC ${LIBNET_ECHO_DIR}
)

# Build the echo client
ADD_EXECUTABLE(echo_client ${LIBNET_ECHO_DIR}/EchoClient.cpp)
TARGET_LINK_LIBRARIES(echo_client libnet)
TARGET_LINK_LIBRARIES(echo_client logger)

TARGET_COMPILE_OPTIONS(echo_client PRIVATE ${CMAKE_COMPILER_FLAG})
TARGET_INCLUDE_DIRECTORIES(
        echo_client
        PUBLIC ${LIBNET_SRC_DIR}
        PUBLIC ${LIBNET_ECHO_DIR}
)

# Build the web server
FILE(GLOB LIBNET_WEB_SERVER_SOURCE RELATIVE ${CMAKE_SOURCE_DIR} "example/WebServer/*.cpp")

ADD_EXECUTABLE(web_server ${LIBNET_WEB_SERVER_SOURCE})
TARGET_LINK_LIBRARIES(web_server libnet)
TARGET_LINK_LIBRARIES(web_server logger)

TARGET_COMPILE_OPTIONS(web_server PRIVATE ${CMAKE_COMPILER_FLAG})
TARGET_INCLUDE_DIRECTORIES(
        web_server
        PUBLIC ${LIBNET_SRC_DIR}
        PUBLIC ${LIBNET_WEBSERVER_DIR}
)

# Build the web client
FILE(GLOB LIBNET_WEB_CLIENT_SOURCE RELATIVE ${CMAKE_SOURCE_DIR} "example/WebServer/test/*.cpp")

ADD_EXECUTABLE(web_client ${LIBNET_WEB_CLIENT_SOURCE})
TARGET_LINK_LIBRARIES(web_client libnet)
TARGET_LINK_LIBRARIES(web_client logger)

TARGET_COMPILE_OPTIONS(web_client PRIVATE ${CMAKE_COMPILER_FLAG})
TARGET_INCLUDE_DIRECTORIES(
        web_client
        PUBLIC ${LIBNET_SRC_DIR}
        PUBLIC ${LIBNET_WEBSERVER_DIR}
        PUBLIC ${LIBNET_WEBSERVER_DIR}/test
)

# # fetch the Catch2 from github
# INCLUDE(FetchContent)

# FetchContent_Declare(
#         Catch2
#         GIT_REPOSITORY git@github.com:catchorg/Catch2.git
#         GIT_TAG        v3.3.0 # or a later release
# )

# FetchContent_MakeAvailable(Catch2)

# # automatic discover and run all the tests with 'ctest' or 'make test'
# LIST(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
# INCLUDE(CTest)
# INCLUDE(Catch)

# ADD_EXECUTABLE(BufferTest ${LIBNET_TEST_DIR}/core/BufferTest.cpp)
# TARGET_LINK_LIBRARIES(BufferTest PRIVATE Catch2::Catch2WithMain libnet)

# ADD_EXECUTABLE(EPollerTest ${LIBNET_TEST_DIR}/core/EPollerTest.cpp)
# TARGET_LINK_LIBRARIES(EPollerTest PRIVATE Catch2::Catch2WithMain libnet logger)

# ADD_EXECUTABLE(AcceptorTest ${LIBNET_TEST_DIR}/core/AcceptorTest.cpp)
# TARGET_LINK_LIBRARIES(AcceptorTest PRIVATE Catch2::Catch2WithMain libnet logger)

# CATCH_DISCOVER_TESTS(BufferTest)
# CATCH_DISCOVER_TESTS(EPollerTest)
# CATCH_DISCOVER_TESTS(AcceptorTest)