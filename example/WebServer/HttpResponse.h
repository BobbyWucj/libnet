#ifndef EXAMPLE_WEBSERVER_HTTPRESPONSE_H
#define EXAMPLE_WEBSERVER_HTTPRESPONSE_H

#include "libnet/Buffer.h"
#include "libnet/base/copyable.h"
#include <map>
#include <string>
#include <unordered_map>


namespace libnet {

class Buffer;

} // namespace libnet


namespace webserver 
{

using libnet::Buffer;
using std::map;
using std::string;

class HttpResponse : public libnet::copyable
{
public:
    enum HttpStatusCode {
        kUnknown,
        k200Ok = 200,
        k301MovedPermanently = 301,
        k400BadRequest = 400,
        k403Forbidden = 403,
        k404NotFound = 404,
        k501NotImplemented = 501
    };

    static std::unordered_map<int, std::string> statusTitleMap;
    static std::unordered_map<int, std::string> statusMessageMap;

    explicit HttpResponse(bool closeConnection)
        : statusCode_(kUnknown),
          closeConnection_(closeConnection)
    { }

    void setStatusCode(const HttpStatusCode &statusCode) 
    { statusCode_ = statusCode; }

    void setStatusMessage(const string &statusMessage) 
    { statusMessage_ = statusMessage; }

    bool closeConnection() const { return closeConnection_; }
    void setCloseConnection(bool closeConnection) 
    { closeConnection_ = closeConnection; }

    void setBody(const string &body) { body_ = body; }

    void addHeader(const string& field, const string& value) {
        headers_[field] = value;
    }

    void eraseHeaders() {
        headers_.clear();
    }

    void setContentType(const string& contentType) { 
        addHeader("Content-Type", contentType); 
    }

    void appendToBuffer(Buffer& output) const;

private:
    map<string, string> headers_;
    HttpStatusCode statusCode_;
    string statusMessage_;
    bool closeConnection_;
    string body_;
};


}

#endif // EXAMPLE_WEBSERVER_HTTPRESPONSE_H
