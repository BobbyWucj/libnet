#ifndef LIBNET_BASE_NONCOPYABLE_H
#define LIBNET_BASE_NONCOPYABLE_H

namespace libnet
{

class noncopyable
{
public:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

} // namespace libnet

#endif // LIBNET_BASE_NONCOPYABLE_H
