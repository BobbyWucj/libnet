#ifndef LIBNET_TIMESTAMP_H
#define LIBNET_TIMESTAMP_H

#include <bits/types/timer_t.h>
#include <chrono>

namespace libnet {

using std::chrono::system_clock;
using namespace std::literals::chrono_literals;

using Nanoseconds  = std::chrono::nanoseconds;
using Microseconds = std::chrono::microseconds;
using Milliseconds = std::chrono::milliseconds;
using Seconds      = std::chrono::seconds;
using Minutes      = std::chrono::minutes;
using Hours        = std::chrono::hours;
using Timestamp    = std::chrono::time_point<system_clock, Nanoseconds>;

namespace clock {

    inline Timestamp now() { return system_clock::now(); }

    inline time_t nowFormated() {
        return system_clock::to_time_t(system_clock::now());
    }

    inline Timestamp nowAfter(Nanoseconds interval) { return now() + interval; }

    inline Timestamp nowBefore(Nanoseconds interval) {
        return now() - interval;
    }

}  // namespace clock

template <typename T> struct IntervalTypeCheckImpl
{
    static constexpr bool value = std::is_same<T, Nanoseconds>::value ||
                                  std::is_same<T, Microseconds>::value ||
                                  std::is_same<T, Milliseconds>::value ||
                                  std::is_same<T, Seconds>::value ||
                                  std::is_same<T, Minutes>::value ||
                                  std::is_same<T, Hours>::value;
};

template <typename T> inline void IntervalTypeCheck(T) {
    static_assert(IntervalTypeCheckImpl<T>::value, "bad interval value");
}

}  // namespace libnet

#endif  // LIBNET_TIMESTAMP_H
