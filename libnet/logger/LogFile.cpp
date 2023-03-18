#include "logger/LogFile.h"
#include <cassert>

using namespace std;
using namespace libnet;
using namespace libnet::logger;

LogFile::LogFile(const string& basename, int flushEveryN)
    : basename_(basename),
      flushEveryN_(flushEveryN),
      count_(0),
      file_(make_unique<AppendFile>(basename))
{ }

void LogFile::append(const char* logline, size_t len) {
    lock_guard<mutex> guard(mutex_);
    append_unlocked(logline, len);
}

void LogFile::flush() {
    lock_guard<mutex> guard(mutex_);
    file_->flush();
}

void LogFile::append_unlocked(const char* logline, size_t len) {
    file_->append(logline, len);
    ++count_;
    if (count_ >= flushEveryN_) {
        count_ = 0;
        file_->flush();
    }
}