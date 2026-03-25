#pragma once

#include <cstdint>

namespace mruntime {

class Clock {
public:
    static uint64_t uptime_ns();
    static uint64_t uptime_us();
    static uint64_t epoch_ms();

    static void sleep_ns(uint64_t ns);
    static void sleep_us(uint64_t us);
    static void sleep_ms(uint64_t ms);

    Clock() = default;
};

}  // namespace mruntime
