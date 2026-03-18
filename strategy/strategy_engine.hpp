#pragma once

#include "core/events.hpp"
#include "orderbook/order_book.hpp"
#include "core/event_queue.hpp"
#include "market_maker_strategy.hpp"
#include "momentum_strategy.hpp"
#include <variant>

namespace hft::strategy {

using StrategyVariant = std::variant<MarketMakerStrategy, MomentumStrategy>;

class StrategyEngine {
public:
    StrategyEngine(hft::core::EventQueue& event_queue, const hft::orderbook::OrderBook& order_book);
    
    void on_market_data(const hft::core::MarketDataEvent& event);
    void on_execution(const hft::core::ExecutionReport& report);
    void update_position(double position);

    void add_strategy(StrategyVariant strategy);

private:
    hft::core::EventQueue& event_queue_;
    const hft::orderbook::OrderBook& order_book_;
    
    std::vector<StrategyVariant> strategies_;
};

} // namespace hft::strategy
