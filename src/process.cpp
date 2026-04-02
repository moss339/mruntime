#include <mruntime/process.h>
#include <mruntime/types.h>
#include <cerrno>
#include <cstring>
#include <sched.h>
#include <sys/resource.h>
#include <unistd.h>
#include <vector>

namespace moss {
namespace mruntime {

void Process::set_priority(int priority) {
    if (priority < -20 || priority > 19) {
        throw RuntimeError("Priority must be between -20 (highest) and 19 (lowest), got: " + std::to_string(priority));
    }

    if (setpriority(PRIO_PROCESS, 0, priority) != 0) {
        throw RuntimeError("Failed to set priority: " + std::string(strerror(errno)));
    }
}

int Process::get_priority() {
    int priority = getpriority(PRIO_PROCESS, 0);
    if (priority == -1 && errno != 0) {
        throw RuntimeError("Failed to get priority: " + std::string(strerror(errno)));
    }
    return priority;
}

void Process::set_cpu_affinity(const std::vector<int>& cores) {
    const uint32_t cpu_count = get_cpu_count();

    for (int core : cores) {
        if (core < 0 || core >= static_cast<int>(cpu_count)) {
            throw RuntimeError("Invalid CPU core: " + std::to_string(core) +
                              ", available cores: 0-" + std::to_string(cpu_count - 1));
        }
    }

    cpu_set_t mask;
    CPU_ZERO(&mask);
    for (int core : cores) {
        CPU_SET(core, &mask);
    }

    if (sched_setaffinity(0, sizeof(mask), &mask) != 0) {
        throw RuntimeError("Failed to set CPU affinity: " + std::string(strerror(errno)));
    }
}

std::vector<int> Process::get_cpu_affinity() {
    const uint32_t cpu_count = get_cpu_count();
    cpu_set_t mask;
    CPU_ZERO(&mask);

    if (sched_getaffinity(0, sizeof(mask), &mask) != 0) {
        throw RuntimeError("Failed to get CPU affinity: " + std::string(strerror(errno)));
    }

    std::vector<int> cores;
    for (uint32_t i = 0; i < cpu_count; ++i) {
        if (CPU_ISSET(i, &mask)) {
            cores.push_back(static_cast<int>(i));
        }
    }
    return cores;
}

uint32_t Process::get_cpu_count() {
    long num = sysconf(_SC_NPROCESSORS_ONLN);
    if (num < 1) {
        num = 1;
    }
    return static_cast<uint32_t>(num);
}

}  // namespace mruntime

}  // namespace moss
