add_executable(echo_server EchoServer.cpp)
target_link_libraries(echo_server libnet)

add_executable(echo_client EchoClient.cpp)
target_link_libraries(echo_client libnet)