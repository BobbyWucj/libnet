#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <strings.h>
#include <sys/socket.h>

#include "Logger.h"
#include "InetAddress.h"

using namespace libnet;

InetAddress::InetAddress(uint16_t port, bool loopback)
{
    bzero(&address_, sizeof(address_));
    address_.sin_family = AF_INET;
    in_addr_t ip = loopback ? INADDR_LOOPBACK : INADDR_ANY;
    address_.sin_addr.s_addr = htonl(ip);
    address_.sin_port = htons(port);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port)
{
    bzero(&address_, sizeof(address_));
    address_.sin_family = AF_INET;
    int ret = ::inet_pton(AF_INET, ip.c_str(), &address_.sin_addr.s_addr);
    if (ret != 1) {
        LOG_SYSFATAL("InetAddress::inet_pton()");
    }
    address_.sin_port = htons(port);
}

std::string InetAddress::toIp() const {
    char buf[INET_ADDRSTRLEN];
    const char* ret = inet_ntop(AF_INET, &address_.sin_addr, buf, sizeof(buf));
    if (ret == nullptr) {
        buf[0] = '\0';
        LOG_SYSERR("InetAddress::inet_ntop()");
    }
    return std::string(buf);
}

uint16_t InetAddress::toPort() const {
    return ntohs(address_.sin_port);
}

std::string InetAddress::toIpPort() const {
    std::string ret = toIp();
    ret.push_back(':');
    return ret.append(std::to_string(toPort()));
}
