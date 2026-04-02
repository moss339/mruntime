/**
 * @file node.h
 * @brief MRuntime Node Implementation
 *
 * MRuntime provides the runtime foundation for MOSS nodes, including
 * process management, scheduling, and communication endpoints.
 */

#ifndef MRUNTIME_NODE_H
#define MRUNTIME_NODE_H

#include "node_state.h"
#include "node_config.h"
#include <mruntime/clock.h>
#include <mdds/mdds.h>

#include <memory>
#include <mutex>
#include <unordered_map>
#include <string>
#include <vector>
#include <functional>

namespace moss {
namespace mruntime {

template<typename T>
class Publisher;

template<typename T>
class Subscriber;

/**
 * @brief Runtime Node Class
 *
 * Represents a running instance in the MOSS system. Each node provides
 * communication endpoints (publishers/subscribers) and lifecycle management.
 *
 * @note This is a low-level runtime node. For high-level API, use mcom::Node.
 *
 * @section lifecycle Node Lifecycle
 * @dot
 * digraph NodeLifecycle {
 *   UNINITIALIZED -> INITIALIZED [label="init()"];
 *   INITIALIZED -> RUNNING [label="start()"];
 *   RUNNING -> STOPPED [label="stop()"];
 *   STOPPED -> DESTROYED [label="destroy()"];
 *   UNINITIALIZED -> DESTROYED [label="destroy()"];
 * }
 * @enddot
 */
class Node : public std::enable_shared_from_this<Node> {
public:
    /**
     * @brief Create a new Node instance
     * @param node_name Unique name for this node
     * @param domain_id DDS domain ID (0-255)
     * @return Shared pointer to the created node
     */
    static std::shared_ptr<Node> create(const std::string& node_name,
                                        mdds::DomainId domain_id = 0);

    /**
     * @brief Construct a Node with configuration
     * @param config Node configuration
     */
    explicit Node(const NodeConfig& config);

    /** @brief Destructor */
    ~Node();

    /** @brief Disable copy construction */
    Node(const Node&) = delete;

    /** @brief Disable copy assignment */
    Node& operator=(const Node&) = delete;

    /**
     * @brief Initialize the node
     * @return true if initialization succeeded
     */
    bool init();

    /**
     * @brief Start the node (begin processing)
     * @return true if start succeeded
     */
    bool start();

    /**
     * @brief Stop the node (pause processing)
     */
    void stop();

    /**
     * @brief Destroy the node (cleanup resources)
     */
    void destroy();

    /**
     * @brief Check if node is running
     * @return true if node is in RUNNING state
     */
    bool is_running() const;

    /**
     * @brief Get current node state
     * @return Current NodeState
     */
    NodeState get_state() const;

    /**
     * @brief Get node name
     * @return Node name string
     */
    const std::string& get_name() const;

    /**
     * @brief Get DDS domain ID
     * @return Domain ID
     */
    mdds::DomainId get_domain_id() const;

    /**
     * @brief Get underlying DDS participant
     * @return Shared pointer to DomainParticipant
     */
    std::shared_ptr<mdds::DomainParticipant> get_participant() const { return participant_; }

    /**
     * @brief Create a publisher for a topic
     * @tparam T Data type for the topic
     * @param topic_name Name of the topic
     * @param qos QoS configuration
     * @return Shared pointer to Publisher
     */
    template<typename T>
    std::shared_ptr<Publisher<T>> create_publisher(const std::string& topic_name,
                                                     const mdds::QoSConfig& qos = mdds::default_qos::publisher());

    /**
     * @brief Create a subscriber for a topic
     * @tparam T Data type for the topic
     * @param topic_name Name of the topic
     * @param callback Callback function for received data
     * @param qos QoS configuration
     * @return Shared pointer to Subscriber
     */
    template<typename T>
    std::shared_ptr<Subscriber<T>> create_subscriber(const std::string& topic_name,
                                                      typename mdds::Subscriber<T>::DataCallback callback,
                                                      const mdds::QoSConfig& qos = mdds::default_qos::subscriber());

private:
    void transition_state(NodeState new_state);
    void validate_state(NodeState expected) const;

    NodeConfig config_;
    NodeState state_{NodeState::UNINITIALIZED};
    mutable std::mutex state_mutex_;

    std::shared_ptr<mdds::DomainParticipant> participant_;

    /** @brief Internal publisher entry */
    struct PublisherEntry {
        std::string topic_name;
        std::shared_ptr<void> publisher;
    };

    /** @brief Internal subscriber entry */
    struct SubscriberEntry {
        std::string topic_name;
        std::shared_ptr<void> subscriber;
    };

    std::vector<PublisherEntry> publishers_;
    std::vector<SubscriberEntry> subscribers_;
    std::mutex endpoints_mutex_;
};

/**
 * @brief Publisher Template Class
 * @tparam T Data type published on this topic
 *
 * Wraps an mdds::Publisher with weak reference to the parent Node.
 */
template<typename T>
class Publisher {
public:
    /** @brief Callback type for data reception */
    using DataCallback = std::function<void(const T& data, uint64_t timestamp)>;

    /** @brief Default constructor */
    Publisher() = default;

    /**
     * @brief Construct Publisher with node and mdds publisher
     * @param node Weak pointer to parent node
     * @param publisher Shared pointer to mdds publisher
     */
    Publisher(std::weak_ptr<Node> node,
              std::shared_ptr<mdds::Publisher<T>> publisher)
        : node_(std::move(node)), publisher_(std::move(publisher)) {}

    /**
     * @brief Write data to the topic
     * @param data Data to write
     * @return true if write succeeded
     */
    bool write(const T& data) {
        if (auto p = publisher_.lock()) {
            return p->write(data);
        }
        return false;
    }

    /**
     * @brief Write data with timestamp
     * @param data Data to write
     * @param timestamp Timestamp in microseconds
     * @return true if write succeeded
     */
    bool write(const T& data, uint64_t timestamp) {
        if (auto p = publisher_.lock()) {
            return p->write(data, timestamp);
        }
        return false;
    }

    /**
     * @brief Get topic name
     * @return Topic name string
     */
    const std::string& get_topic_name() const {
        static std::string empty;
        if (auto p = publisher_.lock()) {
            return p->get_topic_name();
        }
        return empty;
    }

    /**
     * @brief Get topic ID
     * @return Topic ID
     */
    mdds::TopicId get_topic_id() const {
        if (auto p = publisher_.lock()) {
            return p->get_topic_id();
        }
        return 0;
    }

    /**
     * @brief Get underlying mdds publisher
     * @return Shared pointer to mdds publisher (may be expired)
     */
    std::shared_ptr<mdds::Publisher<T>> get_mdds_publisher() const {
        return publisher_.lock();
    }

private:
    std::weak_ptr<Node> node_;
    std::weak_ptr<mdds::Publisher<T>> publisher_;
};

/**
 * @brief Subscriber Template Class
 * @tparam T Data type subscribed to
 *
 * Wraps an mdds::Subscriber with weak reference to the parent Node.
 */
template<typename T>
class Subscriber {
public:
    /** @brief Default constructor */
    Subscriber() = default;

    /**
     * @brief Construct Subscriber with node and mdds subscriber
     * @param node Weak pointer to parent node
     * @param subscriber Shared pointer to mdds subscriber
     */
    Subscriber(std::weak_ptr<Node> node,
               std::shared_ptr<mdds::Subscriber<T>> subscriber)
        : node_(std::move(node)), subscriber_(std::move(subscriber)) {}

    /**
     * @brief Set data callback
     * @param callback Callback function
     */
    void set_callback(typename mdds::Subscriber<T>::DataCallback callback) {
        if (auto s = subscriber_.lock()) {
            s->set_callback(std::move(callback));
        }
    }

    /**
     * @brief Read data from subscription
     * @param data Output data
     * @param timestamp Optional output timestamp
     * @return true if data was read
     */
    bool read(T& data, uint64_t* timestamp = nullptr) {
        if (auto s = subscriber_.lock()) {
            return s->read(data, timestamp);
        }
        return false;
    }

    /**
     * @brief Check if data is available
     * @return true if data is available
     */
    bool has_data() const {
        if (auto s = subscriber_.lock()) {
            return s->has_data();
        }
        return false;
    }

    /**
     * @brief Get topic name
     * @return Topic name string
     */
    const std::string& get_topic_name() const {
        static std::string empty;
        if (auto s = subscriber_.lock()) {
            return s->get_topic_name();
        }
        return empty;
    }

    /**
     * @brief Get topic ID
     * @return Topic ID
     */
    mdds::TopicId get_topic_id() const {
        if (auto s = subscriber_.lock()) {
            return s->get_topic_id();
        }
        return 0;
    }

    /**
     * @brief Get underlying mdds subscriber
     * @return Shared pointer to mdds subscriber (may be expired)
     */
    std::shared_ptr<mdds::Subscriber<T>> get_mdds_subscriber() const {
        return subscriber_.lock();
    }

private:
    std::weak_ptr<Node> node_;
    std::weak_ptr<mdds::Subscriber<T>> subscriber_;
};

template<typename T>
std::shared_ptr<Publisher<T>> Node::create_publisher(
    const std::string& topic_name, const mdds::QoSConfig& qos) {

    std::lock_guard<std::mutex> lock(endpoints_mutex_);

    if (state_ != NodeState::INITIALIZED && state_ != NodeState::RUNNING) {
        throw NodeStateException(state_, NodeState::INITIALIZED);
    }

    auto mdds_publisher = participant_->create_publisher<T>(topic_name, qos);
    auto publisher = std::make_shared<Publisher<T>>(weak_from_this(), mdds_publisher);

    PublisherEntry entry;
    entry.topic_name = topic_name;
    entry.publisher = mdds_publisher;
    publishers_.push_back(std::move(entry));

    return publisher;
}

template<typename T>
std::shared_ptr<Subscriber<T>> Node::create_subscriber(
    const std::string& topic_name,
    typename mdds::Subscriber<T>::DataCallback callback,
    const mdds::QoSConfig& qos) {

    std::lock_guard<std::mutex> lock(endpoints_mutex_);

    if (state_ != NodeState::INITIALIZED && state_ != NodeState::RUNNING) {
        throw NodeStateException(state_, NodeState::INITIALIZED);
    }

    auto mdds_subscriber = participant_->create_subscriber<T>(topic_name, std::move(callback), qos);
    auto subscriber = std::make_shared<Subscriber<T>>(weak_from_this(), mdds_subscriber);

    SubscriberEntry entry;
    entry.topic_name = topic_name;
    entry.subscriber = mdds_subscriber;
    subscribers_.push_back(std::move(entry));

    return subscriber;
}

}  // namespace mruntime

}  // namespace moss
#endif  // MRUNTIME_NODE_H
