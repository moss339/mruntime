#pragma once

#include <stdexcept>
#include <string>

namespace mruntime {

class RuntimeError : public std::runtime_error {
public:
    explicit RuntimeError(const std::string& msg) : std::runtime_error(msg) {}
};

}  // namespace mruntime
