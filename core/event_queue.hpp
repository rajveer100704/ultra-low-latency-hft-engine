#pragma once

#include "ring_buffer.hpp"
#include "events.hpp"

namespace hft::core {

class EventQueue {
public:
    static constexpr size_t DefaultSize = 1024;

    bool push(const Event& event) {
        total_pushed_.fetch_add(1, std::memory_order_relaxed);
        return queue_.push(event);
    }

    std::optional<Event> pop() {
        return queue_.pop();
    }

    size_t size() const {
        return queue_.size();
    }

    uint64_t total_pushed() const {
        return total_pushed_.load(std::memory_order_relaxed);
    }

private:
    RingBuffer<Event, DefaultSize> queue_;
    std::atomic<uint64_t> total_pushed_{0};
};

} // namespace hft::core
