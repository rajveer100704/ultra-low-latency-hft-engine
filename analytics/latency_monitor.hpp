#pragma once

#include <vector>
#include <algorithm>
#include <mutex>
#include <cstdint>
#include "core/latency_tracker.hpp"

namespace hft::analytics {

class LatencyMonitor {
public:
    struct Stats {
        uint64_t p50_ns;
        uint64_t p90_ns;
        uint64_t p99_ns;
        uint64_t avg_ns;
        uint64_t count;
    };

    void record_signal_to_order(uint64_t signal_ts, uint64_t order_ts) {
        if (signal_ts > 0 && order_ts > signal_ts) {
            std::lock_guard<std::mutex> lock(mutex_);
            signal_to_order_samples_.push_back(order_ts - signal_ts);
        }
    }

    void record_order_to_fill(uint64_t order_ts, uint64_t fill_ts) {
        if (order_ts > 0 && fill_ts > order_ts) {
            std::lock_guard<std::mutex> lock(mutex_);
            order_to_fill_samples_.push_back(fill_ts - order_ts);
        }
    }

    void record_total_latency(uint64_t signal_ts, uint64_t fill_ts) {
        if (signal_ts > 0 && fill_ts > signal_ts) {
            std::lock_guard<std::mutex> lock(mutex_);
            total_samples_.push_back(fill_ts - signal_ts);
        }
    }

    Stats get_signal_to_order_stats() { return compute_stats(signal_to_order_samples_); }
    Stats get_order_to_fill_stats() { return compute_stats(order_to_fill_samples_); }
    Stats get_total_latency_stats() { return compute_stats(total_samples_); }

private:
    Stats compute_stats(std::vector<uint64_t>& samples) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (samples.empty()) return {0, 0, 0, 0, 0};

        std::sort(samples.begin(), samples.end());
        
        uint64_t sum = 0;
        for (auto s : samples) sum += s;

        Stats s;
        s.count = samples.size();
        s.avg_ns = sum / s.count;
        s.p50_ns = samples[s.count * 0.50];
        s.p90_ns = samples[s.count * 0.90];
        s.p99_ns = samples[s.count * 0.99];
        return s;
    }

    std::vector<uint64_t> signal_to_order_samples_;
    std::vector<uint64_t> order_to_fill_samples_;
    std::vector<uint64_t> total_samples_;
    std::mutex mutex_;
};

} // namespace hft::analytics
