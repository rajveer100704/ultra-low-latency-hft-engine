#pragma once

#include "core/events.hpp"
#include <string>
#include <cstdint>

namespace hft::strategy {

/**
 * @brief Base interface for all trading strategies.
 * Allows the StrategyEngine to manage multiple strategies concurrently.
 */
class IStrategy {
public:
    virtual ~IStrategy() = default;

    // Called on every market data update (trades, book updates)
    virtual void on_market_data(const hft::core::MarketDataEvent& event) = 0;

    // Called when a trade execution is confirmed for this strategy
    virtual void on_execution(const hft::core::ExecutionReport& report) = 0;

    // Called when position is updated (can be strategy-specific or global)
    virtual void update_position(double position) = 0;

    virtual std::string get_name() const = 0;

    // Strategies can emit orders via the engine's queue (provided during init)
};

} // namespace hft::strategy
