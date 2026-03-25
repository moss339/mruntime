#pragma once

#include <mruntime/types.h>
#include <cstdint>
#include <vector>

namespace mruntime {

class Process {
public:
    static void set_priority(int priority);
    static int get_priority();

    static void set_cpu_affinity(const std::vector<int>& cores);
    static std::vector<int> get_cpu_affinity();

    static uint32_t get_cpu_count();

    Process() = default;
};

}  // namespace mruntime
