#pragma once

#include "core/latency_tracker.hpp"
#include "orderbook/order_book.hpp"
#include <cstdint>
#include <string>
#include <vector>

#include "pnl/pnl_engine.hpp"
#include <map>

namespace hft::ui {

class TerminalUI {
public:
    TerminalUI(const hft::orderbook::OrderBook& order_book);
    ~TerminalUI();

    void update(double total_pnl, double total_position, 
                const std::map<uint32_t, hft::pnl::StrategyStats>& strategy_stats,
                uint64_t p99_latency_ns,
                size_t queue_depth,
                uint64_t events_per_sec,
                const std::vector<hft::core::OrderRequest>& open_orders,
                const std::vector<hft::core::ExecutionReport>& trades);
    bool should_close() const;

private:
    const hft::orderbook::OrderBook& order_book_;
    bool should_close_ = false;
};

} // namespace hft::ui
