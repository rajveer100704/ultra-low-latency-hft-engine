#include "strategy_engine.hpp"
#include <variant> // Added for std::variant and std::visit

namespace hft::strategy {

// Assuming StrategyVariant is defined in strategy_engine.hpp as something like:
// using StrategyVariant = std::variant<ConcreteStrategy1, ConcreteStrategy2, ...>;
// And strategies_ member is now std::vector<StrategyVariant>

StrategyEngine::StrategyEngine(hft::core::EventQueue& event_queue, const hft::orderbook::OrderBook& order_book)
    : event_queue_(event_queue), order_book_(order_book) {}

void StrategyEngine::add_strategy(StrategyVariant strategy) { // Changed parameter type
    strategies_.push_back(std::move(strategy));
}

void StrategyEngine::on_market_data(const hft::core::MarketDataEvent& event) {
    for (auto& s : strategies_) {
        std::visit([&](auto& strategy) { // Using std::visit for static dispatch
            strategy.on_market_data(event);
        }, s);
    }
}

void StrategyEngine::on_execution(const hft::core::ExecutionReport& report) {
    // In a multi-strategy system, we use strategy_id for routing
    // For simplicity, we broadcast to all, but real systems use a map
    for (auto& s : strategies_) {
        std::visit([&](auto& strategy) { // Using std::visit for static dispatch
            strategy.on_execution(report);
        }, s);
    }
}

void StrategyEngine::update_position(double position) {
    for (auto& s : strategies_) {
        std::visit([&](auto& strategy) { // Using std::visit for static dispatch
            strategy.update_position(position);
        }, s);
    }
}

} // namespace hft::strategy
