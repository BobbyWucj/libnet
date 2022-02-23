#ifndef LIBNET_BASE_LOGFILE_H
#define LIBNET_BASE_LOGFILE_H

#include <cstddef>
#include <memory>
#include <string>
#include <mutex>
#include "noncopyable.h"
#include "FileUtil.h"

namespace libnet {
namespace logger {

class LogFile : noncopyable 
{
public:
    explicit LogFile(const std::string& basename, int flushEveryN = 1024);

    void append(const char* logline, size_t len);
    void flush();
    bool rollFile();

private:
    // will flush stream after append every flushEveryN_ times
    void append_unlocked(const char* logline, size_t len);

    const std::string basename_;
    const int flushEveryN_;

    int count_;
    std::mutex mutex_;
    std::unique_ptr<AppendFile> file_;
};

} // namespace logger
} // namespace libnet

#endif // LIBNET_BASE_LOGFILE_H
