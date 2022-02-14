
aux_source_directory(. SRC)

add_library(libnet STATIC ${SRC})
target_link_libraries(libnet pthread)

file(GLOB HEADERS "*.*")

install(TARGETS libnet DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include)



