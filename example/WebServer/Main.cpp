#include "WebServer.h"
#include "HttpParser.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "libnet/EventLoop.h"
#include "libnet/InetAddress.h"
#include "libnet/base/Logger.h"
#include <bits/types/FILE.h>
#include <string>
#include <sys/stat.h>

using namespace webserver;
using namespace libnet;

int main(int argc, char* argv[])
{
    Logger::setLogLevel(Logger::INFO);
    size_t numThreads = 1; // need to be larger than 0
    if (argc > 1) {
        numThreads = static_cast<size_t>(atoi(argv[1]));
    }
    EventLoop loop;
    WebServer server(&loop, InetAddress(8090));
    server.setNumThreads(numThreads);
    server.start();
    loop.loop();
}
