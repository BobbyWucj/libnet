#ifndef LIBNET_BUFFER_H
#define LIBNET_BUFFER_H

#include <algorithm>
#include <cassert>
#include <cstring>
#include <endian.h>
#include <string>
#include <vector>

namespace libnet {

class Buffer
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize  = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize),
          readerIndex_(kCheapPrepend),
          writerIndex_(kCheapPrepend) {
        assert(readableBytes() == 0);
        assert(writableBytes() == initialSize);
        assert(prependableBytes() == kCheapPrepend);
    }

    size_t readableBytes() const { return writerIndex_ - readerIndex_; }
    size_t writableBytes() const { return buffer_.size() - writerIndex_; }
    size_t prependableBytes() const { return readerIndex_; }

    void swap(Buffer& rhs) {
        buffer_.swap(rhs.buffer_);
        std::swap(readerIndex_, rhs.readerIndex_);
        std::swap(writerIndex_, rhs.writerIndex_);
    }

    void ensureWritableBytes(size_t len) {
        if (writableBytes() < len) {
            makeSpace(len);
        }
        assert(writableBytes() >= len);
    }

    char*       beginWrite() { return begin() + writerIndex_; }
    const char* beginWrite() const { return begin() + writerIndex_; }

    void hasWritten(size_t len) {
        assert(len <= writableBytes());
        writerIndex_ += len;
    }

    const char* peek() const { return begin() + readerIndex_; }

    const char* findCRLF() const { return findCRLF(peek()); }

    const char* findCRLF(const char* start) const {
        assert(peek() <= start);
        assert(start <= beginWrite());
        const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
        return crlf == beginWrite() ? nullptr : crlf;
    }

    const char* findEOL() const {
        const void* eol = memchr(peek(), '\n', readableBytes());
        return static_cast<const char*>(eol);
    }

    const char* findEOL(const char* start) const {
        assert(peek() <= start);
        assert(start <= beginWrite());
        const void* eol = memchr(start, '\n', beginWrite() - start);
        return static_cast<const char*>(eol);
    }

    // retrieve : 移动 index，相当于移除数据
    void retrieveAll() {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }

    void retrieve(size_t len) {
        assert(len <= readableBytes());
        if (len < readableBytes()) {
            readerIndex_ += len;
        }
        else {
            retrieveAll();
        }
    }

    void retrieveUntil(const char* end) {
        assert(peek() <= end);
        assert(end <= beginWrite());
        retrieve(end - peek());
    }

    template <typename T> void retrieveInt() { retrieve(sizeof(T)); }

    std::string retrieveAsString(size_t len) {
        assert(len <= readableBytes());
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    std::string retrieveAllAsString() {
        return retrieveAsString(readableBytes());
    }

    void append(const char* data, size_t len) {
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        hasWritten(len);
    }

    void append(const std::string& data) {
        append(data.c_str(), data.length());
    }
    void append(const void* data, size_t len) {
        append(static_cast<const char*>(data), len);
    }

    template <typename T> void appendInt(T x) {
        T be = htobe(x);
        append(&be, sizeof(be));
    }

    // read : 窥探数据并移除
    template <typename T> T readInt() {
        T result = peekInt<T>();
        retrieveInt<T>();
        return result;
    }

    // peek : 窥探数据，不移除
    template <typename T> T peekInt() const {
        assert(readableBytes() >= sizeof(T));
        T be = 0;
        ::memcpy(&be, peek(), sizeof(be));
        return betoh(be);
    }

    // prepend : 将 data 插入前置空间
    void prepend(const void* data, size_t len) {
        assert(len <= prependableBytes());
        readerIndex_ -= len;
        auto d = static_cast<const char*>(data);
        std::copy(d, d + len, begin() + readerIndex_);
    }

    void prepend(const std::string& str) { prepend(str.c_str(), str.size()); }

    template <typename T> void prependInt(T x) {
        T be = htobe(x);
        prepend(&be, sizeof(be));
    }

    ssize_t readFd(int fd, int* savedErrno);

private:
    char*       begin() { return &*buffer_.begin(); }
    const char* begin() const { return &*buffer_.begin(); }

    void makeSpace(size_t len) {
        if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
            buffer_.resize(writerIndex_ + len);
        }
        else {
            assert(kCheapPrepend < readerIndex_);
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_, begin() + writerIndex_,
                      begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
            assert(readable == readableBytes());
        }
    }

    template <typename T> T htobe(T value) const {
        if constexpr (sizeof(T) == 1) {
            return value;
        }
        else if constexpr (sizeof(T) == 2) {
            return htobe16(value);
        }
        else if constexpr (sizeof(T) == 4) {
            return htobe32(value);
        }
        else if constexpr (sizeof(T) == 8) {
            return htobe64(value);
        }
        else {
            static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 ||
                              sizeof(T) == 8,
                          "Unsupported size");
        }
    }

    template <typename T> T betoh(T value) const {
        if constexpr (sizeof(T) == 1) {
            return value;
        }
        else if constexpr (sizeof(T) == 2) {
            return be16toh(value);
        }
        else if constexpr (sizeof(T) == 4) {
            return be32toh(value);
        }
        else if constexpr (sizeof(T) == 8) {
            return be64toh(value);
        }
        else {
            static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 ||
                              sizeof(T) == 8,
                          "Unsupported size");
        }
    }

    std::vector<char> buffer_;
    size_t            readerIndex_;
    size_t            writerIndex_;

    static const char kCRLF[];
};

}  // namespace libnet

#endif  // LIBNET_BUFFER_H
