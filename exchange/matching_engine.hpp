#pragma once

#include "core/events.hpp"
#include "core/event_queue.hpp"
#include "orderbook/order_book.hpp"
#include <vector>
#include <mutex>
#include <cstdint>

namespace hft::exchange {

class MatchingEngine {
public:
    MatchingEngine(hft::core::EventQueue& event_queue);
    
    // Process incoming order from OMS
    void process_order(const hft::core::OrderRequest& order);
    
    // Update internal book with real market data to provide liquidity
    void update_market_price(const hft::core::MarketDataEvent& md);
    
    // Process CancelOrder from strategy
    void cancel_order(uint64_t order_id);

private:
    void match_market_order(const hft::core::OrderRequest& order);
    void match_limit_order(const hft::core::OrderRequest& order);
    void emit_execution(const hft::core::OrderRequest& order, double price, double quantity, hft::core::OrderStatus status, uint64_t signal_ts, uint64_t order_ts);

    hft::core::EventQueue& event_queue_;
    
    // We use a simplified version of our optimized order book for simulated liquidity
    // In this "simulated exchange", we match against the best available price from real market data
    double current_best_bid_ = 0.0;
    double current_best_ask_ = 0.0;
    double current_bid_qty_ = 0.0;
    double current_ask_qty_ = 0.0;
    
    uint64_t next_trade_id_ = 1;
    std::vector<hft::core::OrderRequest> resting_orders_;
    std::mutex mutex_;
};

} // namespace hft::exchange
