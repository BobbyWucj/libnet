#include "WebServer.h"
#include "HttpParser.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "core/EventLoop.h"
#include "core/InetAddress.h"
#include "logger/Logger.h"
#include <bits/types/FILE.h>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/stat.h>

using namespace webserver;
using namespace libnet;

int main(int argc, char* argv[])
{
    Logger::setLogLevel(Logger::INFO);
    size_t numThreads = 1; // need to be larger than 0
    bool disableReusePort = false;
    if (argc > 1) {
        for(int i = 1; i < argc; ++i) {
            const char* argument = argv[i];
            if(strcmp(argument, "-t") == 0) {
                if (++i == argc) {
                    LOG_SYSFATAL << "main() : argv error!";
                }
                numThreads = static_cast<size_t>(atoi(argv[i]));
                // if (numThreads < 0) {
                //     LOG_SYSERR << "main() : numThreads less than 0!";
                //     numThreads = 1;
                // }
            } else if(strcmp(argument, "-r") == 0) {
                disableReusePort = true;
            } else {
                LOG_ERROR << "main() : argv not recognized!";
            }
        }
    }
    EventLoop loop;
    WebServer server(&loop, InetAddress(8090));
    if (disableReusePort) {
        server.disableReusePort();
    }
    server.setNumThreads(numThreads);
    server.start();
    loop.loop();
}
