#include "logger/Logger.h"
#include "logger/AsyncLogging.h"
#include <thread>
#include <cassert>
#include <sys/time.h>
#include <sys/syscall.h>
#include <unistd.h>

using namespace libnet;
using namespace libnet::logger;

static pthread_once_t once_control_ = PTHREAD_ONCE_INIT;
static AsyncLogging *AsyncLogger_;

namespace libnet {
namespace logger {

__thread int t_cachedTid = 0;

int tid() {
    if (t_cachedTid == 0) {
        t_cachedTid = static_cast<int>(syscall(SYS_gettid));
    }
    return t_cachedTid;
}

std::string Logger::logFileName_ = "./WebServer.log";

#ifndef NDEBUG
Logger::LogLevel g_logLevel = Logger::DEBUG;
#else
Logger::LogLevel g_logLevel = Logger::INFO;
#endif

const char* LogLevelName[Logger::NUM_LOG_LEVELS] =
{
    "TRACE ",
    "DEBUG ",
    "INFO  ",
    "WARN  ",
    "ERROR ",
    "FATAL ",
};

void once_init() {
    AsyncLogger_ = new AsyncLogging(Logger::getLogFileName());
    AsyncLogger_->start(); 
}

void output(const char* msg, size_t len) {
    pthread_once(&once_control_, once_init);
    AsyncLogger_->append(msg, len);
}

} // namespace logger
} // namespace libnet

void Logger::formatTime() {
    struct timeval tv;
    time_t time;
    char str_t[26] = {0};
    gettimeofday (&tv, NULL);
    time = tv.tv_sec;
    struct tm* p_time = localtime(&time);   
    strftime(str_t, 26, "%Y-%m-%d %H:%M:%S\n", p_time);
    stream_ << str_t;
}

Logger::Logger(const char *fileName, int line, LogLevel level)
  : fileName_(fileName),
    line_(line),
    level_(level)
{ 
    formatTime();
    stream_ << tid() << ' ' << LogLevelName[level];
    if (level_ == FATAL) {
        abort();
    }
}

Logger::~Logger()
{
    stream_ << " -- " << fileName_ << ':' << line_ << '\n';
    const LogStream::Buffer& buf(stream().buffer());
    output(buf.data(), buf.length());
}

void Logger::setLogLevel(LogLevel level) {
    g_logLevel = level;
}
