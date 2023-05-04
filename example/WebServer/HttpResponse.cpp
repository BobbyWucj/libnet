#include "HttpResponse.h"
#include <cstring>

using namespace webserver;
using namespace libnet;

std::unordered_map<int, std::string> HttpResponse::statusTitleMap{
    { 200, "OK" },          { 301, "Moved Permanently" },
    { 400, "Bad Request" }, { 403, "Forbidden" },
    { 404, "Not Found" },   { 501, "Not Implemented" }
};

std::unordered_map<int, std::string> HttpResponse::statusMessageMap{
    { 200, "Everything fine" },
    { 301, "The requested file has beed moved permanently" },
    { 400,
      "Your request has bad syntax or is inherently impossible to staisfy" },
    { 403, "You do not have permission to get file form this server" },
    { 404, "The requested file was not found on this server" },
    { 501, "The requested feature is not yet supported, so stay tuned" }
};

void HttpResponse::appendToBuffer(Buffer& output) const {
    char buf[128];
    snprintf(buf, sizeof(buf), "HTTP/1.1 %d %s", statusCode_,
             statusTitleMap[statusCode_].c_str());
    output.append(buf, strlen(buf));

    // if (!statusMessage_.empty()) {
    //     output.append(statusMessage_);
    // }
    // else {
    //     output.append(statusMessageMap[statusCode_]);
    // }

    output.append("\r\n");

    if (closeConnection_) {
        output.append("Connection: close\r\n");
    }
    else {
        snprintf(buf, sizeof(buf), "Content-Length: %zd\r\n", body_.size());
        output.append(buf);
        output.append("Connection: Keep-Alive\r\n");
    }

    for (const auto& header : headers_) {
        output.append(header.first);
        output.append(": ");
        output.append(header.second);
        output.append("\r\n");
    }

    output.append("\r\n");
    output.append(body_);
}
