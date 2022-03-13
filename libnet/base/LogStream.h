#ifndef LIBNET_BASE_LOGSTREAM_H
#define LIBNET_BASE_LOGSTREAM_H

#include <cassert>
#include <string.h>
#include <string>
#include "noncopyable.h"

namespace libnet {
namespace logger {

class AsyncLogging;
const int kSmallBuffer = 4000; // 4KB
const int kLargeBuffer = 4000 * 1000; // 4MB

template <int SIZE>
class FixedBuffer : noncopyable 
{
public:
    FixedBuffer() : cur_(data_) {}

    ~FixedBuffer() {}

    void append(const char* buf, size_t len) {
        if (avail() > len) {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }

    const char* data() const { return data_; }
    size_t length() const { return static_cast<size_t>(cur_ - data_); }

    char* current() { return cur_; }
    size_t avail() const { return static_cast<size_t>(end() - cur_); }
    void add(size_t len) { cur_ += len; }

    void reset() { cur_ = data_; }
    void bzero() { memset(data_, 0, sizeof(data_)); }

private:
    const char* end() const { return data_ + sizeof(data_); }

    char data_[SIZE];
    char* cur_;
};

class LogStream : noncopyable 
{
public:
    using Buffer = FixedBuffer<kSmallBuffer>;

    LogStream& operator<<(bool v) {
        buffer_.append(v ? "1" : "0", 1);
        return *this;
    }

    LogStream& operator<<(short);
    LogStream& operator<<(unsigned short);
    LogStream& operator<<(int);
    LogStream& operator<<(unsigned int);
    LogStream& operator<<(long);
    LogStream& operator<<(unsigned long);
    LogStream& operator<<(long long);
    LogStream& operator<<(unsigned long long);

    LogStream& operator<<(const void*);

    LogStream& operator<<(float v) {
        *this << static_cast<double>(v);
        return *this;
    }
    LogStream& operator<<(double);
    LogStream& operator<<(long double);

    LogStream& operator<<(char v) {
        buffer_.append(&v, 1);
        return *this;
    }

    LogStream& operator<<(const char* str) {
        if (str)
            buffer_.append(str, strlen(str));
        else
            buffer_.append("(null)", 6);
        return *this;
    }

    LogStream& operator<<(const unsigned char* str) {
        return operator<<(reinterpret_cast<const char*>(str));
    }

    LogStream& operator<<(const std::string& v) {
        buffer_.append(v.c_str(), v.size());
        return *this;
    }

    void append(const char* data, size_t len) { buffer_.append(data, len); }
    const Buffer& buffer() const { return buffer_; }
    void resetBuffer() { buffer_.reset(); }

private:
    void staticCheck();

    template <typename T>
    void formatInteger(T);

    Buffer buffer_;

    static const int kMaxNumericSize = 32;
};

} // namespace libnet
} // namespace logger

#endif // LIBNET_BASE_LOGSTREAM_H
