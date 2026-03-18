#pragma once

#include <map>
#include <string>
#include <vector>
#include <mutex>
#include "core/events.hpp"

namespace hft::orderbook {

struct Level {
    double price;
    double quantity;
};

class OrderBook {
public:
    static constexpr int MaxLevels = 100;
    
    void apply_trade(const hft::core::MarketDataEvent& trade);
    
    // For ncurses UI
    std::vector<Level> get_top_bids(int n) const;
    std::vector<Level> get_top_asks(int n) const;

    // Microstructure Metrics
    double get_imbalance() const { return imbalance_.load(std::memory_order_relaxed); }
    double get_buy_volume() const { return buy_volume_.load(std::memory_order_relaxed); }
    double get_sell_volume() const { return sell_volume_.load(std::memory_order_relaxed); }
    double get_spread() const { return spread_.load(std::memory_order_relaxed); }

private:
    // Cache-optimized storage: contiguous arrays
    Level bid_levels_[MaxLevels];
    Level ask_levels_[MaxLevels];
    int num_bids_ = 0;
    int num_asks_ = 0;
    
    mutable std::mutex mutex_;
    double last_price_ = 0.0;

    // Metrics
    std::atomic<double> imbalance_{0.0};
    std::atomic<double> buy_volume_{0.0};
    std::atomic<double> sell_volume_{0.0};
    std::atomic<double> spread_{0.0};
};

} // namespace hft::orderbook
