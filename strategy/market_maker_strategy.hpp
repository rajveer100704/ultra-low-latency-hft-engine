#pragma once

#include "istrategy.hpp"
#include "orderbook/order_book.hpp"
#include <chrono>
#include <cmath>
#include <cstring>
#include <spdlog/spdlog.h>
#include "core/latency_tracker.hpp"

namespace hft::strategy {

class MarketMakerStrategy {
public:
    MarketMakerStrategy(hft::core::EventQueue& event_queue, const hft::orderbook::OrderBook& order_book)
        : event_queue_(event_queue), order_book_(order_book) {}

    void on_market_data(const hft::core::MarketDataEvent& event) {
        auto top_bids = order_book_.get_top_bids(1);
        auto top_asks = order_book_.get_top_asks(1);

        if (top_bids.empty() || top_asks.empty()) return;

        double mid_price = (top_bids[0].price + top_asks[0].price) / 2.0;
        auto now = std::chrono::steady_clock::now();

        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_order_time_).count() < 50) {
            return;
        }

        // Strategy Logic
        double imbalance = (top_bids[0].quantity - top_asks[0].quantity) / (top_bids[0].quantity + top_asks[0].quantity);
        double target_bid = mid_price - (target_spread_ / 2.0) + (imbalance * beta_) - (current_position_ * alpha_);
        double target_ask = mid_price + (target_spread_ / 2.0) + (imbalance * beta_) - (current_position_ * alpha_);

        // Ensure we don't cross the book
        target_bid = std::min(target_bid, top_bids[0].price);
        target_ask = std::max(target_ask, top_asks[0].price);

        // Discretize
        double tick = 1.0; 
        target_bid = std::floor(target_bid / tick) * tick;
        target_ask = std::ceil(target_ask / tick) * tick;

        if (std::abs(target_bid - last_bid_price_) > 0.01 || std::abs(target_ask - last_ask_price_) > 0.01) {
            cancel_all();
            if (current_position_ < max_position_) submit_order(hft::core::OrderSide::Buy, target_bid);
            if (current_position_ > -max_position_) submit_order(hft::core::OrderSide::Sell, target_ask);
            
            last_bid_price_ = target_bid;
            last_ask_price_ = target_ask;
            last_order_time_ = now;
        }
    }

    void on_execution(const hft::core::ExecutionReport& report) {
        if (report.status == hft::core::OrderStatus::Filled || report.status == hft::core::OrderStatus::Partial) {
            // Internal tracking if needed
        }
    }

    void update_position(double position) {
        current_position_ = position;
    }

    std::string get_name() const { return "MarketMakerV1"; }

private:
    void submit_order(hft::core::OrderSide side, double price) {
        hft::core::Event ev;
        ev.type = hft::core::EventType::OrderRequest;
        auto& req = ev.order_request;
        req.signal_ts = hft::core::LatencyTracker::now_ns();
        req.strategy_id = 1; // MarketMaker ID
        req.order_id = next_order_id_++;
        std::memcpy(req.symbol, "BTCUSDT", 7);
        req.side = side;
        req.price = price;
        req.quantity = 0.1;
        req.type = hft::core::OrderType::Limit;
        
        active_orders_.push_back(req.order_id);
        event_queue_.push(ev);
    }

    void cancel_all() {
        for (auto id : active_orders_) {
            hft::core::Event ev;
            ev.type = hft::core::EventType::CancelOrder;
            ev.cancel_order.order_id = id;
            event_queue_.push(ev);
        }
        active_orders_.clear();
    }

    hft::core::EventQueue& event_queue_;
    const hft::orderbook::OrderBook& order_book_;
    double current_position_ = 0.0;
    double max_position_ = 10.0;
    double alpha_ = 0.01;
    double beta_ = 1.0;
    double target_spread_ = 2.0;
    double last_bid_price_ = 0.0;
    double last_ask_price_ = 0.0;
    std::chrono::steady_clock::time_point last_order_time_{};
    std::vector<uint64_t> active_orders_;
    uint64_t next_order_id_ = 1000000; // Offset for MM
};

} // namespace hft::strategy
