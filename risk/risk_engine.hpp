#pragma once

#include "core/events.hpp"
#include <cstdint>

namespace hft::risk {

class RiskEngine {
public:
    RiskEngine(double max_pos = 10.0) : max_position_(max_pos) {}
    bool check_order(const hft::core::OrderRequest& order);
    void update_position(double quantity);

private:
    double current_position_ = 0.0;
    double max_position_; // BTC
};

} // namespace hft::risk
