#ifndef EXAMPLE_WEBSERVER_HTTPPARSER_H
#define EXAMPLE_WEBSERVER_HTTPPARSER_H

#include "HttpRequest.h"
#include "core/Timestamp.h"
#include "utils/copyable.h"

namespace libnet {

class Buffer;

}

namespace webserver {

using libnet::Buffer;
using libnet::Timestamp;

class HttpParser : public libnet::copyable
{
public:
    enum HttpRequestParseState {
        kExpectRequestLine,
        kExpectHeaders,
        kExpectBody,
        kGotAll,
    };

    HttpParser() : state_(kExpectRequestLine) {}

    bool parseRequest(Buffer& buf);

    bool gotAll() const { return state_ == kGotAll; }

    void reset() {
        state_ = kExpectRequestLine;
        HttpRequest dummy;
        request_.swap(dummy);
    }

    HttpRequest request() const { return request_; }
    HttpRequest& request() { return request_; }

private:
    bool processRequestLine(const char* begin, const char* end);

    HttpRequestParseState state_;
    HttpRequest request_;
};

}  // namespace webserver

#endif  // EXAMPLE_WEBSERVER_HTTPPARSER_H
