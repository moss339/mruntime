/**
 * @file node_state.h
 * @brief Node State Management for MRuntime
 */

#ifndef MRUNTIME_NODE_STATE_H
#define MRUNTIME_NODE_STATE_H

#include <stdexcept>
#include <string>

namespace moss {
namespace mruntime {

/**
 * @brief Node Lifecycle States
 *
 * Represents the various states a Node can be in during its lifecycle.
 */
enum class NodeState {
    UNINITIALIZED = 0,  /**< @brief Node created but not initialized */
    INITIALIZED = 1,    /**< @brief Node initialized successfully */
    RUNNING = 2,        /**< @brief Node is running and processing */
    STOPPED = 3,         /**< @brief Node has been stopped */
    DESTROYED = 4        /**< @brief Node has been destroyed */
};

/**
 * @brief Base exception for node-related errors
 */
class NodeException : public std::runtime_error {
public:
    /** @brief Construct exception with message */
    explicit NodeException(const std::string& message) : std::runtime_error(message) {}
};

/**
 * @brief Exception thrown when node state transition is invalid
 *
 * This exception indicates that an operation was attempted when the node
 * was not in the expected state.
 */
class NodeStateException : public NodeException {
public:
    /**
     * @brief Construct state exception
     * @param current Current node state
     * @param expected Expected node state for the operation
     */
    NodeStateException(NodeState current, NodeState expected);

    /** @brief Get the current state */
    NodeState get_current_state() const { return current_; }

    /** @brief Get the expected state */
    NodeState get_expected_state() const { return expected_; }

private:
    NodeState current_;   /**< @brief Current state when exception was thrown */
    NodeState expected_;  /**< @brief Expected state for the failed operation */
};

/**
 * @brief Convert NodeState to string representation
 * @param state NodeState to convert
 * @return C-string representation of the state
 */
inline const char* to_string(NodeState state) {
    switch (state) {
        case NodeState::UNINITIALIZED: return "UNINITIALIZED";
        case NodeState::INITIALIZED:   return "INITIALIZED";
        case NodeState::RUNNING:       return "RUNNING";
        case NodeState::STOPPED:       return "STOPPED";
        case NodeState::DESTROYED:     return "DESTROYED";
        default:                        return "UNKNOWN";
    }
}

}  // namespace mruntime

}  // namespace moss
#endif  // MRUNTIME_NODE_STATE_H
