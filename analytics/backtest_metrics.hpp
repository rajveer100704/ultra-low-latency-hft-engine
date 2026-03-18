#pragma once

#include <vector>
#include <string>
#include "core/events.hpp"

namespace hft::analytics {

struct BacktestMetrics {
    double total_pnl = 0.0;
    int trade_count = 0;
    double win_rate = 0.0;
    double max_drawdown = 0.0;
    double sharp_ratio = 0.0; // Placeholder for future expansion
};

} // namespace hft::analytics
