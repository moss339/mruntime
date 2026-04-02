#pragma once

#include <mruntime/process.h>
#include <mruntime/clock.h>
#include <memory>

namespace moss {
namespace mruntime {

class Runtime : public std::enable_shared_from_this<Runtime> {
public:
    struct Config {
        int priority{0};
        std::vector<int> cpu_affinity;
    };

    static std::shared_ptr<Runtime> create(const Config& config);
    static std::shared_ptr<Runtime> create();

    const Config& config() const { return config_; }

    Process& process() { return process_; }
    Clock& clock() { return clock_; }

private:
    explicit Runtime(const Config& config);

    Config config_;
    Process process_;
    Clock clock_;
};

}  // namespace mruntime
}  // namespace moss
