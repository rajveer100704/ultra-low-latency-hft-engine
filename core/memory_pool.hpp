#pragma once

#include <vector>
#include <mutex>
#include <array>

namespace hft::core {

/**
 * @brief A simple, thread-safe (via pre-allocation) memory pool for POD types.
 * Designed to be used in the hot path without any dynamic allocation.
 */
template <typename T, size_t Size>
class MemoryPool {
public:
    MemoryPool() : next_free_(0) {}

    T* allocate() {
        size_t idx = next_free_.fetch_add(1, std::memory_order_relaxed);
        if (idx >= Size) {
            return nullptr; // Pool exhausted
        }
        return &pool_[idx];
    }

    void reset() {
        next_free_.store(0, std::memory_order_release);
    }

private:
    std::array<T, Size> pool_;
    std::atomic<size_t> next_free_{0};
};

} // namespace hft::core
