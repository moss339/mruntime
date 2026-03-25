#include <mruntime/node.h>

namespace mruntime {

NodeStateException::NodeStateException(NodeState current, NodeState expected)
    : NodeException("Node state error: expected " + std::string(to_string(expected)) +
                    " but current state is " + std::string(to_string(current)))
    , current_(current), expected_(expected) {
}

Node::Node(const NodeConfig& config)
    : config_(config) {
}

Node::~Node() {
    if (state_ != NodeState::DESTROYED && state_ != NodeState::UNINITIALIZED) {
        destroy();
    }
}

std::shared_ptr<Node> Node::create(const std::string& node_name, mdds::DomainId domain_id) {
    NodeConfig config;
    config.node_name = node_name;
    config.domain_id = domain_id;
    return std::shared_ptr<Node>(new Node(config));
}

bool Node::init() {
    std::lock_guard<std::mutex> lock(state_mutex_);

    if (state_ != NodeState::UNINITIALIZED) {
        return false;
    }

    participant_ = mdds::DomainParticipant::create(config_.domain_id);
    if (!participant_) {
        return false;
    }

    transition_state(NodeState::INITIALIZED);
    return true;
}

bool Node::start() {
    std::lock_guard<std::mutex> lock(state_mutex_);

    if (state_ != NodeState::INITIALIZED) {
        return false;
    }

    if (!participant_->start()) {
        return false;
    }

    transition_state(NodeState::RUNNING);
    return true;
}

void Node::stop() {
    std::lock_guard<std::mutex> lock(state_mutex_);

    if (state_ != NodeState::RUNNING) {
        return;
    }

    participant_->stop();
    transition_state(NodeState::STOPPED);
}

void Node::destroy() {
    std::lock_guard<std::mutex> lock(state_mutex_);

    if (state_ == NodeState::DESTROYED) {
        return;
    }

    if (state_ == NodeState::RUNNING) {
        stop();
    }

    {
        std::lock_guard<std::mutex> endpoints_lock(endpoints_mutex_);
        publishers_.clear();
        subscribers_.clear();
    }

    participant_.reset();
    transition_state(NodeState::DESTROYED);
}

bool Node::is_running() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return state_ == NodeState::RUNNING;
}

NodeState Node::get_state() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return state_;
}

const std::string& Node::get_name() const {
    return config_.node_name;
}

mdds::DomainId Node::get_domain_id() const {
    return config_.domain_id;
}

void Node::transition_state(NodeState new_state) {
    state_ = new_state;
}

void Node::validate_state(NodeState expected) const {
    if (state_ != expected) {
        throw NodeStateException(state_, expected);
    }
}

}  // namespace mruntime
