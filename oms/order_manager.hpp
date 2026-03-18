#pragma once

#include "core/events.hpp"
#include "core/event_queue.hpp"
#include "risk/risk_engine.hpp"
#include <unordered_map>
#include <mutex>

namespace hft::oms {

class OrderManager {
public:
    OrderManager(hft::core::EventQueue& event_queue, hft::risk::RiskEngine& risk_engine);
    
    // Process new order from strategy
    void handle_order_request(const hft::core::OrderRequest& request);
    
    // Update order state based on execution report from exchange
    void handle_execution_report(const hft::core::ExecutionReport& report);

    std::vector<hft::core::OrderRequest> get_active_orders() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<hft::core::OrderRequest> orders;
        for (auto const& [id, req] : active_orders_) {
            orders.push_back(req);
        }
        return orders;
    }

private:
    bool validate_order(const hft::core::OrderRequest& request);

    hft::core::EventQueue& event_queue_;
    hft::risk::RiskEngine& risk_engine_;
    
    // Map of order_id to active orders
    std::unordered_map<uint64_t, hft::core::OrderRequest> active_orders_;
    std::mutex mutex_;
};

} // namespace hft::oms
