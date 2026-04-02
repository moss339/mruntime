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

class Node : public std::enable_shared_from_this<Node> {
public:
    static std::shared_ptr<Node> create(const std::string& node_name,
                                        mdds::DomainId domain_id = 0);

    explicit Node(const NodeConfig& config);
    ~Node();

    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;

    bool init();
    bool start();
    void stop();
    void destroy();

    bool is_running() const;
    NodeState get_state() const;
    const std::string& get_name() const;
    mdds::DomainId get_domain_id() const;
    std::shared_ptr<mdds::DomainParticipant> get_participant() const { return participant_; }

    template<typename T>
    std::shared_ptr<Publisher<T>> create_publisher(const std::string& topic_name,
                                                     const mdds::QoSConfig& qos = mdds::default_qos::publisher());

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

    struct PublisherEntry {
        std::string topic_name;
        std::shared_ptr<void> publisher;
    };
    struct SubscriberEntry {
        std::string topic_name;
        std::shared_ptr<void> subscriber;
    };
    std::vector<PublisherEntry> publishers_;
    std::vector<SubscriberEntry> subscribers_;
    std::mutex endpoints_mutex_;
};

template<typename T>
class Publisher {
public:
    using DataCallback = std::function<void(const T& data, uint64_t timestamp)>;

    Publisher() = default;

    Publisher(std::weak_ptr<Node> node,
              std::shared_ptr<mdds::Publisher<T>> publisher)
        : node_(std::move(node)), publisher_(std::move(publisher)) {}

    bool write(const T& data) {
        if (auto p = publisher_.lock()) {
            return p->write(data);
        }
        return false;
    }

    bool write(const T& data, uint64_t timestamp) {
        if (auto p = publisher_.lock()) {
            return p->write(data, timestamp);
        }
        return false;
    }

    const std::string& get_topic_name() const {
        static std::string empty;
        if (auto p = publisher_.lock()) {
            return p->get_topic_name();
        }
        return empty;
    }

    mdds::TopicId get_topic_id() const {
        if (auto p = publisher_.lock()) {
            return p->get_topic_id();
        }
        return 0;
    }

    std::shared_ptr<mdds::Publisher<T>> get_mdds_publisher() const {
        return publisher_.lock();
    }

private:
    std::weak_ptr<Node> node_;
    std::weak_ptr<mdds::Publisher<T>> publisher_;
};

template<typename T>
class Subscriber {
public:
    Subscriber() = default;

    Subscriber(std::weak_ptr<Node> node,
               std::shared_ptr<mdds::Subscriber<T>> subscriber)
        : node_(std::move(node)), subscriber_(std::move(subscriber)) {}

    void set_callback(typename mdds::Subscriber<T>::DataCallback callback) {
        if (auto s = subscriber_.lock()) {
            s->set_callback(std::move(callback));
        }
    }

    bool read(T& data, uint64_t* timestamp = nullptr) {
        if (auto s = subscriber_.lock()) {
            return s->read(data, timestamp);
        }
        return false;
    }

    bool has_data() const {
        if (auto s = subscriber_.lock()) {
            return s->has_data();
        }
        return false;
    }

    const std::string& get_topic_name() const {
        static std::string empty;
        if (auto s = subscriber_.lock()) {
            return s->get_topic_name();
        }
        return empty;
    }

    mdds::TopicId get_topic_id() const {
        if (auto s = subscriber_.lock()) {
            return s->get_topic_id();
        }
        return 0;
    }

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
