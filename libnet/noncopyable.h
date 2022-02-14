#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H

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

#endif // NONCOPYABLE_H
