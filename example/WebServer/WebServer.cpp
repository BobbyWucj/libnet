#include "WebServer.h"
#include "HttpParser.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "core/Buffer.h"
#include "core/Callbacks.h"
#include "core/TcpConnection.h"
#include "core/Timestamp.h"
#include "logger/Logger.h"

#include <any>
#include <cstddef>
#include <fcntl.h>
#include <functional>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace webserver;
using namespace libnet;

extern const char favicon[555];
const std::string root{ "/root" };

namespace webserver {
namespace mime {

    std::unordered_map<std::string, std::string> mimeMap{
        { ".html", "text/html" },         { ".avi", "video/x-msvideo" },
        { ".bmp", "image/bmp" },          { ".c", "text/plain" },
        { ".doc", "application/msword" }, { ".gif", "image/gif" },
        { ".gz", "application/x-gzip" },  { ".htm", "text/html" },
        { ".ico", "image/x-icon" },       { ".jpg", "image/jpeg" },
        { ".png", "image/png" },          { ".txt", "text/plain" },
        { ".mp3", "audio/mp3" },          { "default", "text/html" }
    };

    std::string getMimeType(const std::string& key) {
        auto res = mimeMap.find(key);
        if (res == mimeMap.end()) {
            return mimeMap["default"];
        }
        return res->second;
    }

}  // namespace mime
}  // namespace webserver

WebServer::WebServer(EventLoop* loop,
                     const InetAddress& listenAddr,
                     const std::string& root)
    : server_(loop, listenAddr),
      httpCallback_(std::bind(&WebServer::onHttp, this, _1, _2)),
      root_(root) {
    server_.setConnectionCallback(
        std::bind(&WebServer::onConnection, this, _1));

    server_.setMessageCallback(std::bind(&WebServer::onMessage, this, _1, _2));
}

void WebServer::start() {
    LOG_WARN << "WebServer starts listening on " << server_.ipPort();
    server_.start();
}

void WebServer::onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
        conn->setContext(std::make_any<HttpParser>());
        LOG_INFO << conn->name() << " connected";
    }
}

void WebServer::onMessage(const TcpConnectionPtr& conn, Buffer& buffer) {
    HttpParser* parser = std::any_cast<HttpParser>(conn->getMutableContext());

    if (!parser->parseRequest(buffer)) {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        LOG_WARN << conn->name() << " bad request shutdowning!";
        conn->shutdown();
    }

    if (parser->gotAll()) {
        onRequest(conn, parser->request());
        parser->reset();
    }
}

void WebServer::onRequest(const TcpConnectionPtr& conn,
                          const HttpRequest& request) {
    const string& connection = request.getHeader("Connection");
    bool close =
        (connection == "close") || (request.version() == HttpRequest::kHttp10 &&
                                    connection != "Keep-Alive");

    HttpResponse response(close);
    httpCallback_(request, &response);

    Buffer buffer;
    response.appendToBuffer(buffer);

    conn->send(buffer);
    if (close) {
        conn->forceClose();
    }
}

void WebServer::onHttp(const HttpRequest& request, HttpResponse* response) {
    LOG_INFO << "Headers " << request.methodString() << " " << request.path();
    const std::map<string, string>& headers = request.headers();
    for (const auto& header : headers) {
        LOG_INFO << header.first << " : " << header.second;
    }

    // 仅支持 Head & Get
    if (request.method() != HttpRequest::kHead &&
        request.method() != HttpRequest::kGet) {
        onError(response, HttpResponse::k501NotImplemented);
        return;
    }

    auto path = request.path();

    if (path == "/") {
        response->setStatusCode(HttpResponse::k200Ok);
        response->setContentType("text/html");
        response->addHeader("WebServer", "Libnet");
        time_t now = clock::nowFormated();
        string nowFormatedString(ctime(&now));
        response->setBody("<html><head><title>WebServer</title></head>"
                          "<body><h1>Hello</h1>Now is " +
                          nowFormatedString + "</body></html>");
        return;
    }
    else if (path == "/favicon.ico") {
        response->setStatusCode(HttpResponse::k200Ok);
        response->setContentType("image/png");
        response->setBody(string(favicon, sizeof(favicon)));
        return;
    }
    else if (path == "/hello") {
        response->setStatusCode(HttpResponse::k200Ok);
        response->setContentType("text/plain");
        response->addHeader("Bobby's WebServer", "Based on Libnet");
        response->setBody("Hello, World!\n");
        return;
    }

    size_t dot = path.find('.');
    string file_type;
    if (dot == string::npos) {
        file_type = mime::getMimeType("default");
    }
    else {
        file_type = mime::getMimeType(path.substr(dot));
    }

    string file_url = root_ + path;
    struct stat file_stat;
    if (stat(file_url.c_str(), &file_stat) < 0) {
        onError(response, HttpResponse::k404NotFound);
        return;
    }
    else {
        response->setContentType(file_type);
        response->addHeader("Bobby's WebServer", "Based on Libnet");
    }

    // methid = Head done
    if (request.method() == HttpRequest::kHead) {
        response->setStatusCode(HttpResponse::k200Ok);
        return;
    }

    int file_fd = ::open(file_url.c_str(), O_RDONLY, 0);
    if (file_fd < 0) {
        response->eraseHeaders();
        onError(response, HttpResponse::k403Forbidden);
        return;
    }

    void* mmap_ret =
        ::mmap(NULL, file_stat.st_size, PROT_READ, MAP_PRIVATE, file_fd, 0);
    ::close(file_fd);
    if (mmap_ret == MAP_FAILED) {
        LOG_ERROR << "WebServer::onHttp(), mmap errno = " << errno;
        response->eraseHeaders();
        onError(response, HttpResponse::k403Forbidden);
        munmap(mmap_ret, file_stat.st_size);
        return;
    }

    char* src = static_cast<char*>(mmap_ret);
    response->setBody(std::string(src, src + file_stat.st_size));

    munmap(mmap_ret, file_stat.st_size);
}

void WebServer::onError(HttpResponse* response,
                        const HttpResponse::HttpStatusCode statusCode) {
    response->setStatusCode(statusCode);
    string body;

    body += "<html><title>哎~出错了!</title>";
    body += "<body bgcolor=\"ffffff\">";
    body +=
        std::to_string(statusCode) + HttpResponse::statusTitleMap[statusCode];
    body += "<hr><em> Bobby's Web Server</em>\n</body></html>";

    response->addHeader("Bobby's WebServer", "Based on Libnet");
    response->setBody(body);
}

const char favicon[555] = {
    '\x89', 'P',    'N',    'G',    '\xD',  '\xA',  '\x1A', '\xA',  '\x0',
    '\x0',  '\x0',  '\xD',  'I',    'H',    'D',    'R',    '\x0',  '\x0',
    '\x0',  '\x10', '\x0',  '\x0',  '\x0',  '\x10', '\x8',  '\x6',  '\x0',
    '\x0',  '\x0',  '\x1F', '\xF3', '\xFF', 'a',    '\x0',  '\x0',  '\x0',
    '\x19', 't',    'E',    'X',    't',    'S',    'o',    'f',    't',
    'w',    'a',    'r',    'e',    '\x0',  'A',    'd',    'o',    'b',
    'e',    '\x20', 'I',    'm',    'a',    'g',    'e',    'R',    'e',
    'a',    'd',    'y',    'q',    '\xC9', 'e',    '\x3C', '\x0',  '\x0',
    '\x1',  '\xCD', 'I',    'D',    'A',    'T',    'x',    '\xDA', '\x94',
    '\x93', '9',    'H',    '\x3',  'A',    '\x14', '\x86', '\xFF', '\x5D',
    'b',    '\xA7', '\x4',  'R',    '\xC4', 'm',    '\x22', '\x1E', '\xA0',
    'F',    '\x24', '\x8',  '\x16', '\x16', 'v',    '\xA',  '6',    '\xBA',
    'J',    '\x9A', '\x80', '\x8',  'A',    '\xB4', 'q',    '\x85', 'X',
    '\x89', 'G',    '\xB0', 'I',    '\xA9', 'Q',    '\x24', '\xCD', '\xA6',
    '\x8',  '\xA4', 'H',    'c',    '\x91', 'B',    '\xB',  '\xAF', 'V',
    '\xC1', 'F',    '\xB4', '\x15', '\xCF', '\x22', 'X',    '\x98', '\xB',
    'T',    'H',    '\x8A', 'd',    '\x93', '\x8D', '\xFB', 'F',    'g',
    '\xC9', '\x1A', '\x14', '\x7D', '\xF0', 'f',    'v',    'f',    '\xDF',
    '\x7C', '\xEF', '\xE7', 'g',    'F',    '\xA8', '\xD5', 'j',    'H',
    '\x24', '\x12', '\x2A', '\x0',  '\x5',  '\xBF', 'G',    '\xD4', '\xEF',
    '\xF7', '\x2F', '6',    '\xEC', '\x12', '\x20', '\x1E', '\x8F', '\xD7',
    '\xAA', '\xD5', '\xEA', '\xAF', 'I',    '5',    'F',    '\xAA', 'T',
    '\x5F', '\x9F', '\x22', 'A',    '\x2A', '\x95', '\xA',  '\x83', '\xE5',
    'r',    '9',    'd',    '\xB3', 'Y',    '\x96', '\x99', 'L',    '\x6',
    '\xE9', 't',    '\x9A', '\x25', '\x85', '\x2C', '\xCB', 'T',    '\xA7',
    '\xC4', 'b',    '1',    '\xB5', '\x5E', '\x0',  '\x3',  'h',    '\x9A',
    '\xC6', '\x16', '\x82', '\x20', 'X',    'R',    '\x14', 'E',    '6',
    'S',    '\x94', '\xCB', 'e',    'x',    '\xBD', '\x5E', '\xAA', 'U',
    'T',    '\x23', 'L',    '\xC0', '\xE0', '\xE2', '\xC1', '\x8F', '\x0',
    '\x9E', '\xBC', '\x9',  'A',    '\x7C', '\x3E', '\x1F', '\x83', 'D',
    '\x22', '\x11', '\xD5', 'T',    '\x40', '\x3F', '8',    '\x80', 'w',
    '\xE5', '3',    '\x7',  '\xB8', '\x5C', '\x2E', 'H',    '\x92', '\x4',
    '\x87', '\xC3', '\x81', '\x40', '\x20', '\x40', 'g',    '\x98', '\xE9',
    '6',    '\x1A', '\xA6', 'g',    '\x15', '\x4',  '\xE3', '\xD7', '\xC8',
    '\xBD', '\x15', '\xE1', 'i',    '\xB7', 'C',    '\xAB', '\xEA', 'x',
    '\x2F', 'j',    'X',    '\x92', '\xBB', '\x18', '\x20', '\x9F', '\xCF',
    '3',    '\xC3', '\xB8', '\xE9', 'N',    '\xA7', '\xD3', 'l',    'J',
    '\x0',  'i',    '6',    '\x7C', '\x8E', '\xE1', '\xFE', 'V',    '\x84',
    '\xE7', '\x3C', '\x9F', 'r',    '\x2B', '\x3A', 'B',    '\x7B', '7',
    'f',    'w',    '\xAE', '\x8E', '\xE',  '\xF3', '\xBD', 'R',    '\xA9',
    'd',    '\x2',  'B',    '\xAF', '\x85', '2',    'f',    'F',    '\xBA',
    '\xC',  '\xD9', '\x9F', '\x1D', '\x9A', 'l',    '\x22', '\xE6', '\xC7',
    '\x3A', '\x2C', '\x80', '\xEF', '\xC1', '\x15', '\x90', '\x7',  '\x93',
    '\xA2', '\x28', '\xA0', 'S',    'j',    '\xB1', '\xB8', '\xDF', '\x29',
    '5',    'C',    '\xE',  '\x3F', 'X',    '\xFC', '\x98', '\xDA', 'y',
    'j',    'P',    '\x40', '\x0',  '\x87', '\xAE', '\x1B', '\x17', 'B',
    '\xB4', '\x3A', '\x3F', '\xBE', 'y',    '\xC7', '\xA',  '\x26', '\xB6',
    '\xEE', '\xD9', '\x9A', '\x60', '\x14', '\x93', '\xDB', '\x8F', '\xD',
    '\xA',  '\x2E', '\xE9', '\x23', '\x95', '\x29', 'X',    '\x0',  '\x27',
    '\xEB', 'n',    'V',    'p',    '\xBC', '\xD6', '\xCB', '\xD6', 'G',
    '\xAB', '\x3D', 'l',    '\x7D', '\xB8', '\xD2', '\xDD', '\xA0', '\x60',
    '\x83', '\xBA', '\xEF', '\x5F', '\xA4', '\xEA', '\xCC', '\x2',  'N',
    '\xAE', '\x5E', 'p',    '\x1A', '\xEC', '\xB3', '\x40', '9',    '\xAC',
    '\xFE', '\xF2', '\x91', '\x89', 'g',    '\x91', '\x85', '\x21', '\xA8',
    '\x87', '\xB7', 'X',    '\x7E', '\x7E', '\x85', '\xBB', '\xCD', 'N',
    'N',    'b',    't',    '\x40', '\xFA', '\x93', '\x89', '\xEC', '\x1E',
    '\xEC', '\x86', '\x2',  'H',    '\x26', '\x93', '\xD0', 'u',    '\x1D',
    '\x7F', '\x9',  '2',    '\x95', '\xBF', '\x1F', '\xDB', '\xD7', 'c',
    '\x8A', '\x1A', '\xF7', '\x5C', '\xC1', '\xFF', '\x22', 'J',    '\xC3',
    '\x87', '\x0',  '\x3',  '\x0',  'K',    '\xBB', '\xF8', '\xD6', '\x2A',
    'v',    '\x98', 'I',    '\x0',  '\x0',  '\x0',  '\x0',  'I',    'E',
    'N',    'D',    '\xAE', 'B',    '\x60', '\x82',
};
