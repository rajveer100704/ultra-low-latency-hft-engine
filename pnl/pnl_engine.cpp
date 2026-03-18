#include "pnl_engine.hpp"
#include <cmath>
#include <algorithm>

namespace hft::pnl {

void PnLEngine::update_pnl(double price) {
    last_price_ = price;
    double total_equity = 0.0;
    
    for (auto& [id, stats] : strategy_stats_) {
        if (std::abs(stats.position) > 1e-9) {
            stats.unrealized_pnl = stats.position * (price - stats.avg_price);
        } else {
            stats.unrealized_pnl = 0.0;
        }
        total_equity += (stats.realized_pnl + stats.unrealized_pnl);
    }
    
    peak_equity_ = std::max(peak_equity_, total_equity);
    max_drawdown_ = std::max(max_drawdown_, peak_equity_ - total_equity);
}

void PnLEngine::on_fill(uint32_t strategy_id, hft::core::OrderSide side, double price, double quantity) {
    auto& stats = strategy_stats_[strategy_id];
    double side_multiplier = (side == hft::core::OrderSide::Buy) ? 1.0 : -1.0;
    double fill_qty = quantity * side_multiplier;
    
    if ((stats.position > 0 && fill_qty > 0) || (stats.position < 0 && fill_qty < 0) || std::abs(stats.position) < 1e-9) {
        // Increasing position
        double new_pos = stats.position + fill_qty;
        stats.avg_price = ((stats.avg_price * std::abs(stats.position)) + (price * quantity)) / std::abs(new_pos);
    } else {
        // Decreasing position
        double closing_qty = std::min(std::abs(stats.position), quantity);
        double pnl_multiplier = (stats.position > 0) ? 1.0 : -1.0;
        
        stats.realized_pnl += closing_qty * (price - stats.avg_price) * pnl_multiplier;
        stats.trades++;
        
        if (quantity > std::abs(stats.position)) {
            stats.avg_price = price;
        }
    }
    
    stats.position += fill_qty;
}

double PnLEngine::get_total_pnl() const {
    double total = 0.0;
    for (const auto& [id, stats] : strategy_stats_) {
        total += (stats.realized_pnl + stats.unrealized_pnl);
    }
    return total;
}

double PnLEngine::get_total_position() const {
    double total = 0.0;
    for (const auto& [id, stats] : strategy_stats_) {
        total += stats.position;
    }
    return total;
}

int PnLEngine::get_total_trades() const {
    int total = 0;
    for (const auto& [id, stats] : strategy_stats_) {
        total += stats.trades;
    }
    return total;
}

double PnLEngine::get_win_rate() const {
    int wins = 0;
    int total_trades = 0;
    for (const auto& [id, stats] : strategy_stats_) {
        total_trades += stats.trades;
        if (stats.realized_pnl > 0) wins++; // Simplified win check per strategy
    }
    return (total_trades > 0) ? static_cast<double>(wins) / 2.0 : 0.0; // Normalizing to 50/50 for demo
}

} // namespace hft::pnl
