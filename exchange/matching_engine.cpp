#include "matching_engine.hpp"
#include "core/latency_tracker.hpp"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <chrono>

namespace hft::exchange {

MatchingEngine::MatchingEngine(hft::core::EventQueue& event_queue)
    : event_queue_(event_queue) {}

void MatchingEngine::update_market_price(const hft::core::MarketDataEvent& md) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    double spread = 0.5; // Tighter spread for more fills 
    current_best_bid_ = md.price - spread / 2.0;
    current_best_ask_ = md.price + spread / 2.0;
    
    // Check resting orders against the new "best" prices
    auto it = resting_orders_.begin();
    while (it != resting_orders_.end()) {
        bool filled = false;
        if (it->side == hft::core::OrderSide::Buy && it->price >= current_best_ask_) {
            emit_execution(*it, current_best_ask_, it->quantity, hft::core::OrderStatus::Filled, it->signal_ts, it->order_ts);
            filled = true;
        } else if (it->side == hft::core::OrderSide::Sell && it->price <= current_best_bid_) {
            emit_execution(*it, current_best_bid_, it->quantity, hft::core::OrderStatus::Filled, it->signal_ts, it->order_ts);
            filled = true;
        }
        
        if (filled) {
            it = resting_orders_.erase(it);
        } else {
            ++it;
        }
    }
}

void MatchingEngine::process_order(const hft::core::OrderRequest& order) {
    auto start = std::chrono::high_resolution_clock::now();
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (order.type == hft::core::OrderType::Market) {
        match_market_order(order);
    } else {
        match_limit_order(order);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    spdlog::info("MatchingEngine: Order {} processed in {} ns", order.order_id, latency);
}

void MatchingEngine::cancel_order(uint64_t order_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // In our simplified engine, we just emit a Cancelled report.
    // Real engines would search the book and remove it.
    // Just enough info to satisfy the OMS cancel report
    hft::core::OrderRequest dummy_req{};
    dummy_req.order_id = order_id;
    emit_execution(dummy_req, 0, 0, hft::core::OrderStatus::Cancelled, 0, 0);
    spdlog::info("MatchingEngine: Cancelled order {}", order_id);
}

void MatchingEngine::match_market_order(const hft::core::OrderRequest& order) {
    double match_price = (order.side == hft::core::OrderSide::Buy) ? current_best_ask_ : current_best_bid_;
    
    if (match_price <= 0) {
        spdlog::warn("MatchingEngine: No liquidity for market order {}", order.order_id);
        emit_execution(order, 0, 0, hft::core::OrderStatus::Rejected, order.signal_ts, order.order_ts);
        return;
    }
    
    // Fill completely at "exchange" price (simplification)
    emit_execution(order, match_price, order.quantity, hft::core::OrderStatus::Filled, order.signal_ts, order.order_ts);
}

void MatchingEngine::match_limit_order(const hft::core::OrderRequest& order) {
    bool can_match = false;
    double match_price = order.price;
    
    if (order.side == hft::core::OrderSide::Buy) {
        if (current_best_ask_ > 0 && order.price >= current_best_ask_) {
            can_match = true;
            match_price = current_best_ask_;
        }
    } else {
        if (current_best_bid_ > 0 && order.price <= current_best_bid_) {
            can_match = true;
            match_price = current_best_bid_;
        }
    }
    
    if (can_match) {
        emit_execution(order, match_price, order.quantity, hft::core::OrderStatus::Filled, order.signal_ts, order.order_ts);
    } else {
        spdlog::info("MatchingEngine: Limit order {} resting in book.", order.order_id);
        resting_orders_.push_back(order);
        emit_execution(order, order.price, 0, hft::core::OrderStatus::New, order.signal_ts, order.order_ts);
    }
}

void MatchingEngine::emit_execution(const hft::core::OrderRequest& order, double price, double quantity, hft::core::OrderStatus status, uint64_t signal_ts, uint64_t order_ts) {
    hft::core::Event ev;
    ev.type = hft::core::EventType::ExecutionReport;
    
    auto& report = ev.execution_report;
    report.order_id = order.order_id;
    report.strategy_id = order.strategy_id; // Added for attribution
    report.trade_id = next_trade_id_++;
    std::copy_n(order.symbol, 16, report.symbol);
    report.side = order.side;
    report.last_price = price;
    report.last_qty = quantity;
    report.cumulative_qty = quantity;
    report.leaves_qty = (status == hft::core::OrderStatus::Filled) ? 0 : order.quantity;
    report.status = status;
    
    // Latency Tracking Propagation
    report.signal_ts = signal_ts;
    report.order_ts = order_ts;
    
    event_queue_.push(ev);
    spdlog::info("MatchingEngine: ExecutionReport for order {} status {}", order.order_id, (int)status);
}

} // namespace hft::exchange
