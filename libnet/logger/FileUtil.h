#ifndef LIBNET_BASE_FILEUTIL_H
#define LIBNET_BASE_FILEUTIL_H

#include "utils/noncopyable.h"
#include <string>

namespace libnet {
namespace logger {

class AppendFile : noncopyable 
{
public:
    explicit AppendFile(std::string filename);
    ~AppendFile();
    // append 会向文件写
    void append(const char *logline, const size_t len);
    void flush();

private:
    size_t write(const char *logline, size_t len);
    FILE *fp_;
    char buffer_[64 * 1024];
};

} // namespace logger
} // namespace libnet

#endif // LIBNET_BASE_FILEUTIL_H
