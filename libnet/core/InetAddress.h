#ifndef LIBNET_INETADDRESS_H
#define LIBNET_INETADDRESS_H

#include "utils/copyable.h"
#include <cstdint>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

namespace libnet {

class InetAddress : private copyable
{
public:
    explicit InetAddress(uint16_t port = 0, bool loopback = false);
    InetAddress(const std::string& ip, uint16_t port);

    const struct sockaddr* getSockaddr() const {
        return reinterpret_cast<const struct sockaddr*>(&address_);
    }
    void setAddress(const struct sockaddr_in& address) { address_ = address; }

    socklen_t getSocklen() const { return sizeof(address_); }

    std::string toIp() const;
    uint16_t    toPort() const;
    std::string toIpPort() const;

private:
    struct sockaddr_in address_;
};

}  // namespace libnet

#endif  // LIBNET_INETADDRESS_H
