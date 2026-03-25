#include <mruntime/mruntime.h>
#include <mruntime/types.h>

namespace mruntime {

Runtime::Runtime(const Config& config)
    : config_(config), process_(), clock_() {
    if (config_.priority != 0) {
        Process::set_priority(config_.priority);
    }

    if (!config_.cpu_affinity.empty()) {
        Process::set_cpu_affinity(config_.cpu_affinity);
    }
}

std::shared_ptr<Runtime> Runtime::create(const Config& config) {
    return std::shared_ptr<Runtime>(new Runtime(config));
}

std::shared_ptr<Runtime> Runtime::create() {
    return create(Config{});
}

}  // namespace mruntime
