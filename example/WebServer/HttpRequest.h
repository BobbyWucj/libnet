#ifndef EXAMPLE_WEBSERVER_HTTPREQUEST_H
#define EXAMPLE_WEBSERVER_HTTPREQUEST_H

#include "core/Timestamp.h"
#include <optional>
#include <stdio.h>
#include <map>
#include <cassert>
#include <string>

namespace webserver {

using std::string;
using std::map;
using libnet::Timestamp;

class HttpRequest
{
public:
    enum Method {
        kInvalid, 
        kGet, 
        kPost, 
        kHead, 
        kPut, 
        kDelete
    };
    enum Version {
        kUnknown, 
        kHttp10, 
        kHttp11
    };

    HttpRequest()
        : method_(kInvalid),
          version_(kUnknown)
    { }

    bool setMethod(const char* start, const char* end) { 
        assert(method_ == kInvalid);
        string m(start, end);
        if (m == "GET") {
            method_ = kGet;
        } else if (m == "POST") {
            method_ = kPost;
        } else if (m == "HEAD") {
            method_ = kHead;
        } else if (m == "PUT") {
            method_ = kPut;
        } else if (m == "DELETE") {
            method_ = kDelete;
        } else {
            method_ = kInvalid;
        }
        return method_ != kInvalid;
    }
    
    Method method() const { return method_; }

    const char* methodString() const {
        const char* result = "UNKNOWN";
        switch (method_) {
        case kGet:
            result = "GET";
            break;
        case kPost:
            result = "POST";
            break;
        case kHead:
            result = "HEAD";
            break;
        case kPut:
            result = "PUT";
            break;
        case kDelete:
            result = "DELETE";
            break;
        default:
            break;
        }
        return result;
    }

    Version version() const { return version_; }
    void setVersion(const Version &version) { version_ = version; }

    const string& path() const { return path_; }
    void setPath(const char* start, const char* end) { 
        path_.assign(start, end); 
    }

    const string& query() const { return query_; }
    void setQuery(const char* start, const char* end) {
        query_.assign(start, end);
    }

    Timestamp receiveTime() const { return receiveTime_; }
    void setReceiveTime(const Timestamp &receiveTime) 
    { receiveTime_ = receiveTime; }

    void addHeader(const char* start, const char* colon, const char* end) {
        string field(start, colon);
        while(colon < end && isspace(*colon)) {
            ++colon;
        }
        
        string value(colon, end);
        while(!value.empty() && isspace(value[value.size() - 1])) {
            value.resize(value.size() - 1);
        }
        headers_[field] = value;
    }

    string getHeader(const string& field) const {
        string header;
        std::map<string, string>::const_iterator it = headers_.find(field);
        if (it != headers_.end()) {
            header = it->second;
        }
        return header;
    }

    map<string, string> headers() const { return headers_; }

    void swap(HttpRequest& rhs) {
        std::swap(method_, rhs.method_);
        std::swap(version_, rhs.version_);
        path_.swap(rhs.path_);
        query_.swap(rhs.query_);
        std::swap(receiveTime_, rhs.receiveTime_);
        headers_.swap(rhs.headers_);
    }

private:
    Method              method_;
    Version             version_;
    string              path_;
    string              query_;
    Timestamp           receiveTime_;
    map<string, string> headers_;
};

} // namespace webserver

#endif // EXAMPLE_WEBSERVER_HTTPREQUEST_H
