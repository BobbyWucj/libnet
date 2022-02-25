SET(LOGGER_SRC
    AsyncLogging.cpp
    FileUtil.cpp
    LogFile.cpp
    Logger.cpp
    LogStream.cpp
)

SET(BASE_HEADERS
    AsyncLogging.h
    copyable.h
    CountDownLatch.h
    FileUtil.h
    LogFile.h
    Logger.h
    LogStream.h
    noncopyable.h)

add_library(logger ${LOGGER_SRC})

set_target_properties(logger PROPERTIES OUTPUT_NAME "logger")

install(TARGETS logger DESTINATION lib)
install(FILES ${BASE_HEADERS} DESTINATION include/logger)
