#include <mruntime/clock.h>
#include <cerrno>
#include <ctime>
#include <time.h>
#include <unistd.h>

namespace mruntime {

uint64_t Clock::uptime_ns() {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0;
    }
    return static_cast<uint64_t>(ts.tv_sec) * 1000000000ULL + ts.tv_nsec;
}

uint64_t Clock::uptime_us() {
    return uptime_ns() / 1000ULL;
}

uint64_t Clock::epoch_ms() {
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
        return 0;
    }
    return static_cast<uint64_t>(ts.tv_sec) * 1000ULL + ts.tv_nsec / 1000000ULL;
}

void Clock::sleep_ns(uint64_t ns) {
    struct timespec ts;
    ts.tv_sec = ns / 1000000000ULL;
    ts.tv_nsec = ns % 1000000000ULL;

    while (clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, &ts) == EINTR) {
    }
}

void Clock::sleep_us(uint64_t us) {
    sleep_ns(us * 1000ULL);
}

void Clock::sleep_ms(uint64_t ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000ULL;
    ts.tv_nsec = (ms % 1000ULL) * 1000000ULL;

    while (clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, &ts) == EINTR) {
    }
}

}  // namespace mruntime
