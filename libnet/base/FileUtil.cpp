#include "FileUtil.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;
using namespace libnet;
using namespace libnet::logger;

AppendFile::AppendFile(string filename)
    : fp_(fopen(filename.c_str(), "ae")) // O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC
{
    // 用户提供缓冲区
    // If BUF is NULL, make STREAM unbuffered.
    // Else make it use SIZE bytes of BUF for buffering.
    setbuffer(fp_, buffer_, sizeof(buffer_));
}

AppendFile::~AppendFile()
{ 
    fclose(fp_); 
}

void AppendFile::append(const char* logline, const size_t len) {
    size_t n = this->write(logline, len);
    size_t remain = len - n;
    while(remain > 0) {
        size_t x = this->write(logline + n, remain);
        if (x == 0) {
            int err = ferror(fp_);
            if (err) 
                fprintf(stderr, "AppendFile::append() failed !\n");
            break;
        }
        n += x;
        remain = len - n;
    }
}

void AppendFile::flush() { 
    fflush(fp_); // Flush STREAM
}

size_t AppendFile::write(const char* logline, size_t len) {
    // fwrite的线程不安全版本，不加锁，速度快
    return fwrite_unlocked(logline, 1, len, fp_);
}