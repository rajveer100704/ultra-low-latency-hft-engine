#pragma once

#include "istrategy.hpp"
#include "orderbook/order_book.hpp"
#include <spdlog/spdlog.h>
#include <cstring>
#include <string>
#include "core/latency_tracker.hpp"

namespace hft::strategy {

/**
 * @brief Simple Momentum Strategy
 * Detects price trends using short/long EMA crossover.
 */
class MomentumStrategy {
private:
    hft::core::EventQueue& event_queue_;
    const hft::orderbook::OrderBook& order_book_;
    double short_ema_ = 0.0;
    double long_ema_ = 0.0;
    uint64_t count_ = 0;
    bool position_open_ = false;
    uint64_t next_order_id_ = 2000000; // Offset for Momentum

    void submit_order(hft::core::OrderSide side, double price) {
        hft::core::Event ev;
        ev.type = hft::core::EventType::OrderRequest;
        auto& req = ev.order_request;
        req.signal_ts = hft::core::LatencyTracker::now_ns();
        req.strategy_id = 2; 
        req.order_id = next_order_id_++;
        std::memcpy(req.symbol, "BTCUSDT", 7);
        req.side = side;
        req.price = price;
        req.quantity = 0.5;
        req.type = hft::core::OrderType::Limit;
        event_queue_.push(ev);
    }

public:
    MomentumStrategy(hft::core::EventQueue& event_queue, const hft::orderbook::OrderBook& order_book)
        : event_queue_(event_queue), order_book_(order_book) {}

    void on_market_data(const hft::core::MarketDataEvent& event) {
        // O(1) Branchless EMA update: EMA = price * alpha + prev_EMA * (1 - alpha)
        constexpr double alpha_10 = 2.0 / (10 + 1);
        constexpr double alpha_50 = 2.0 / (50 + 1);
        
        // Initializing if first tick
        short_ema_ = (count_ == 0) ? event.price : (event.price * alpha_10) + (short_ema_ * (1.0 - alpha_10));
        long_ema_ = (count_ == 0) ? event.price : (event.price * alpha_50) + (long_ema_ * (1.0 - alpha_50));
        count_++;

        if (count_ < 50) return; // Warm up

        // Branchless signal logic
        bool bullish = (short_ema_ > long_ema_ * 1.0001) && !position_open_;
        bool bearish = (short_ema_ < long_ema_ * 0.9999) && position_open_;

        if (bullish) {
            submit_order(hft::core::OrderSide::Buy, event.price + 1.0);
            position_open_ = true;
        } else if (bearish) {
            submit_order(hft::core::OrderSide::Sell, event.price - 1.0);
            position_open_ = false;
        }
    }

    void on_execution(const hft::core::ExecutionReport& report) {}
    void update_position(double position) {}
    std::string get_name() const { return "MomentumAlpha"; }
};

} // namespace hft::strategy
