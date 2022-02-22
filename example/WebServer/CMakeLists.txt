aux_source_directory(. WEB_SERVER_SRC)

add_executable(webserver ${WEB_SERVER_SRC})
target_link_libraries(webserver libnet)