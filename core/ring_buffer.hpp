#pragma once

#include <vector>
#include <atomic>
#include <optional>

namespace hft::core {

template <typename T, size_t Size>
class RingBuffer {
public:
    RingBuffer() : head_(0), tail_(0) {
        buffer_.resize(Size);
    }

    bool push(const T& item) {
        size_t head = head_.load(std::memory_order_relaxed);
        size_t next_head = (head + 1) % Size;
        if (next_head == tail_.load(std::memory_order_acquire)) {
            return false; // Buffer full
        }
        buffer_[head] = item;
        head_.store(next_head, std::memory_order_release);
        return true;
    }

    std::optional<T> pop() {
        size_t tail = tail_.load(std::memory_order_relaxed);
        if (tail == head_.load(std::memory_order_acquire)) {
            return std::nullopt; // Buffer empty
        }
        T item = buffer_[tail];
        tail_.store((tail + 1) % Size, std::memory_order_release);
        return item;
    }

    size_t size() const {
        size_t head = head_.load(std::memory_order_acquire);
        size_t tail = tail_.load(std::memory_order_acquire);
        if (head >= tail) return head - tail;
        return Size - (tail - head);
    }

private:
    std::vector<T> buffer_;
    std::atomic<size_t> head_;
    std::atomic<size_t> tail_;
};

} // namespace hft::core
