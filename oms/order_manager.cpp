#include "order_manager.hpp"
#include "core/latency_tracker.hpp"
#include <spdlog/spdlog.h>

namespace hft::oms {

OrderManager::OrderManager(hft::core::EventQueue& event_queue, hft::risk::RiskEngine& risk_engine)
    : event_queue_(event_queue), risk_engine_(risk_engine) {}

void OrderManager::handle_order_request(const hft::core::OrderRequest& request) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (validate_order(request)) {
        spdlog::info("OMS: Order {} validated and accepted.", request.order_id);
        active_orders_[request.order_id] = request;
        
        // Forward to Matching Engine (via queue)
        hft::core::Event ev;
        ev.type = hft::core::EventType::ValidatedOrder;
        ev.order_request = request;
        ev.order_request.order_ts = hft::core::LatencyTracker::now_ns();
        event_queue_.push(ev);
    } else {
        spdlog::warn("OMS: Order {} rejected by risk check.", request.order_id);
    }
}

void OrderManager::handle_execution_report(const hft::core::ExecutionReport& report) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = active_orders_.find(report.order_id);
    if (it != active_orders_.end()) {
        if (report.status == hft::core::OrderStatus::Filled || 
            report.status == hft::core::OrderStatus::Cancelled ||
            report.status == hft::core::OrderStatus::Rejected) {
            active_orders_.erase(it);
        }
        
        spdlog::info("OMS: Execution report for Order {}: Status {}", report.order_id, (int)report.status);
    }
}

bool OrderManager::validate_order(const hft::core::OrderRequest& request) {
    // Basic structural validation
    if (request.quantity <= 0) return false;
    if (request.type == hft::core::OrderType::Limit && request.price <= 0) return false;
    
    // Forward to RiskEngine for position checks
    return risk_engine_.check_order(request);
}

} // namespace hft::oms
