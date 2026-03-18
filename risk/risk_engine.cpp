#include "risk_engine.hpp"
#include <spdlog/spdlog.h>

namespace hft::risk {

bool RiskEngine::check_order(const hft::core::OrderRequest& order) {
    double side_sign = (order.side == hft::core::OrderSide::Buy) ? 1.0 : -1.0;
    if (std::abs(current_position_ + (order.quantity * side_sign)) > max_position_) {
        spdlog::warn("Risk: Max position limit reached ({:.2f}). Blocking order.", max_position_);
        return false;
    }
    return true;
}

void RiskEngine::update_position(double quantity) {
    current_position_ += quantity;
    spdlog::info("Risk: Position updated to {:.4f}", current_position_);
}

} // namespace hft::risk
