#include "execution_engine.hpp"
#include <spdlog/spdlog.h>

namespace hft::execution {

ExecutionEngine::ExecutionEngine(hft::core::EventQueue& event_queue) : event_queue_(event_queue) {}

void ExecutionEngine::execute(const hft::core::MarketDataEvent& order) {
    spdlog::info("Execution: Placing order for {:.4f} @ {:.2f}", order.quantity, order.price);
    
    // In a real system, this would send an order to Binance API
    // For now, we simulate an immediate fill
    // hft::core::Event fill;
    // fill.type = hft::core::EventType::ExecutionReport;
    // ... fill details ...
    // event_queue_.push(fill);
}

} // namespace hft::execution
