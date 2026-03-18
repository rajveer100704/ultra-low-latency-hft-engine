#pragma once

#include <atomic>
#include <chrono>
#include <vector>
#include <numeric>
#include <cstdint>

namespace hft::core {

class LatencyTracker {
public:
    static inline uint64_t now_ns() {
        return std::chrono::steady_clock::now().time_since_epoch().count();
    }

    struct Stats {
        double avg_ns;
        uint64_t max_ns;
        uint64_t count;
    };

    void record_feed_to_parse(uint64_t ns) { feed_to_parse_.record(ns); }
    void record_parse_to_book(uint64_t ns) { parse_to_book_.record(ns); }
    void record_book_to_strategy(uint64_t ns) { book_to_strategy_.record(ns); }
    void record_strategy_to_exec(uint64_t ns) { strategy_to_exec_.record(ns); }
    void record_end_to_end(uint64_t ns) { end_to_end_.record(ns); }

    Stats get_feed_to_parse_stats() { return feed_to_parse_.get_stats(); }
    Stats get_parse_to_book_stats() { return parse_to_book_.get_stats(); }
    Stats get_book_to_strategy_stats() { return book_to_strategy_.get_stats(); }
    Stats get_strategy_to_exec_stats() { return strategy_to_exec_.get_stats(); }
    Stats get_end_to_end_stats() { return end_to_end_.get_stats(); }

private:
    struct Metric {
        std::atomic<uint64_t> total_ns{0};
        std::atomic<uint64_t> max_ns{0};
        std::atomic<uint64_t> count{0};

        void record(uint64_t ns) {
            total_ns.fetch_add(ns, std::memory_order_relaxed);
            count.fetch_add(1, std::memory_order_relaxed);
            
            uint64_t current_max = max_ns.load(std::memory_order_relaxed);
            while (ns > current_max && !max_ns.compare_exchange_weak(current_max, ns, std::memory_order_relaxed));
        }

        Stats get_stats() {
            uint64_t c = count.load(std::memory_order_relaxed);
            if (c == 0) return {0.0, 0, 0};
            return {static_cast<double>(total_ns.load(std::memory_order_relaxed)) / c, 
                    max_ns.load(std::memory_order_relaxed), c};
        }
    };

    Metric feed_to_parse_;
    Metric parse_to_book_;
    Metric book_to_strategy_;
    Metric strategy_to_exec_;
    Metric end_to_end_;
};

} // namespace hft::core
