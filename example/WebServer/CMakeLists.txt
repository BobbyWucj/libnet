SET(WEB_SERVER_SRC
    HttpParser.cpp
    HttpResponse.cpp
    WebServer.cpp
    Main.cpp)

add_executable(webserver ${WEB_SERVER_SRC})
target_link_libraries(webserver libnet)