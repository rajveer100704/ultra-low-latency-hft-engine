#pragma once

#include "core/events.hpp"
#include "core/event_queue.hpp"

namespace hft::execution {

class ExecutionEngine {
public:
    ExecutionEngine(hft::core::EventQueue& event_queue);
    void execute(const hft::core::MarketDataEvent& order);

private:
    hft::core::EventQueue& event_queue_;
};

} // namespace hft::execution
