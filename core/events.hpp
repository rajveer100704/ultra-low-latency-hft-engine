#pragma once

#include <string>
#include <chrono>
#include <cstdint>
#include <cstring>

namespace hft::core {

enum class EventType {
    MarketData,
    OrderRequest,
    ValidatedOrder,
    OrderResponse,
    ExecutionReport,
    RiskCheck,
    PnLUpdate,
    CancelOrder
};

enum class OrderSide { Buy, Sell };
enum class OrderType { Limit, Market };
enum class OrderStatus { New, Partial, Filled, Cancelled, Rejected };

struct MarketDataEvent {
    char symbol[16];
    double price;
    double quantity;
    long long exchange_ts;
    bool is_buyer_maker;
};

struct OrderRequest {
    uint64_t order_id;
    uint32_t strategy_id; // id of strategy that generated this order
    char symbol[16];
    OrderSide side;
    OrderType type;
    double price;
    double quantity;
    
    // Latency tracking
    uint64_t signal_ts; // signal generation time
    uint64_t order_ts;  // order management system time
};

struct ExecutionReport {
    uint64_t order_id;
    uint32_t strategy_id; // Added for multi-strategy attribution
    uint64_t trade_id;
    char symbol[16];
    OrderSide side;
    double last_price;
    double last_qty;
    double cumulative_qty;
    double leaves_qty;
    OrderStatus status;
    
    // Latency tracking
    uint64_t signal_ts;
    uint64_t order_ts;
};

struct CancelOrderReq {
    uint64_t order_id;
};

struct alignas(64) Event {
    EventType type;
    
    // Low-latency instrumentation timestamps
    uint64_t receive_ts;   
    uint64_t parse_ts;     
    uint64_t book_ts;      
    uint64_t strategy_ts;  
    uint64_t execution_ts; 
    
    union {
        MarketDataEvent market_data;
        OrderRequest order_request;
        ExecutionReport execution_report;
        CancelOrderReq cancel_order;
    };
};

} // namespace hft::core
