#pragma once

#include "core/events.hpp"
#include <map>
#include <string>
#include <cstdint>

namespace hft::pnl {

struct StrategyStats {
    double realized_pnl = 0.0;
    double unrealized_pnl = 0.0;
    double position = 0.0;
    double avg_price = 0.0;
    int trades = 0;
};

class PnLEngine {
public:
    void update_pnl(double price);
    void on_fill(uint32_t strategy_id, hft::core::OrderSide side, double price, double quantity);
    
    double get_total_pnl() const;
    double get_total_position() const;
    double get_max_drawdown() const { return max_drawdown_; }
    int get_total_trades() const;
    double get_win_rate() const;
    
    const std::map<uint32_t, StrategyStats>& get_strategy_stats() const { return strategy_stats_; }

private:
    std::map<uint32_t, StrategyStats> strategy_stats_;
    double last_price_ = 0.0;
    
    double peak_equity_ = 0.0;
    double max_drawdown_ = 0.0;
};

} // namespace hft::pnl
