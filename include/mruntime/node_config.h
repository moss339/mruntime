#ifndef MRUNTIME_NODE_CONFIG_H
#define MRUNTIME_NODE_CONFIG_H

#include <cstdint>
#include <string>
#include <vector>

namespace moss {
namespace mruntime {

struct NodeConfig {
    std::string node_name;
    uint8_t domain_id{0};
    bool enable_multicast_discovery{false};

    NodeConfig() = default;

    explicit NodeConfig(const std::string& name, uint8_t domain = 0)
        : node_name(name), domain_id(domain) {}
};

}  // namespace mruntime

}  // namespace moss
#endif  // MRUNTIME_NODE_CONFIG_H
