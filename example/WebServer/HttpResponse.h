#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include "libnet/Buffer.h"
#include "libnet/copyable.h"
#include <map>
#include <string>


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
        k404NotFound = 404,
    };

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

#endif // HTTPRESPONSE_H
