#ifndef LIBNET_BASE_LOGGER_H
#define LIBNET_BASE_LOGGER_H

#include <string.h>
#include <string>
#include "LogStream.h"

namespace libnet {
namespace logger {

class AsyncLogging;

class Logger 
{
public:
    enum LogLevel {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS,
    };

    Logger(const char *fileName, int line, LogLevel level);
    ~Logger();

    LogStream &stream() { return stream_; }

    static LogLevel logLevel();
    static void setLogLevel(LogLevel level);

    static std::string getLogFileName() { return logFileName_; }
    static void setLogFileName(std::string logFileName) 
    { logFileName_ = logFileName; }

private:
    void formatTime();

    static std::string logFileName_; // Logger fileName

    LogStream stream_;
    std::string fileName_; // fileName of the Log caller
    int line_;
    LogLevel level_;
};

#define LOG Logger(__FILE__, __LINE__).stream()

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::logLevel() {
    return g_logLevel;
}

} // namespace logger

using namespace libnet::logger;

#define LOG_TRACE if (Logger::logLevel() <= Logger::TRACE) \
  Logger(__FILE__, __LINE__, Logger::TRACE).stream()
#define LOG_DEBUG if (Logger::logLevel() <= Logger::DEBUG) \
  Logger(__FILE__, __LINE__, Logger::DEBUG).stream()
#define LOG_INFO if (Logger::logLevel() <= Logger::INFO) \
  Logger(__FILE__, __LINE__, Logger::INFO).stream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::FATAL).stream()
#define LOG_SYSERR Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_SYSFATAL Logger(__FILE__, __LINE__, Logger::FATAL).stream()

} // namespace libnet

#endif // LIBNET_BASE_LOGGER_H
