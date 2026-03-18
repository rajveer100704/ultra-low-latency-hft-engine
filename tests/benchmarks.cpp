#include <iostream>
#include <chrono>
#include <vector>
#include "orderbook/order_book.hpp"
#include "core/event_queue.hpp"

void benchmark_order_book() {
    hft::orderbook::OrderBook book;
    hft::core::MarketDataEvent ev{"BTCUSDT", 60000.0, 1.0, 123456, false};
    
    auto start = std::chrono::steady_clock::now();
    const int iterations = 1000000;
    
    for (int i = 0; i < iterations; ++i) {
        ev.price += 0.01;
        book.apply_trade(ev);
    }
    
    auto end = std::chrono::steady_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    
    std::cout << "OrderBook Update: " << iterations << " iterations in " 
              << diff / 1e6 << " ms (" << diff / iterations << " ns/op)" << std::endl;
}

void benchmark_event_queue() {
    hft::core::EventQueue queue;
    hft::core::Event ev;
    ev.type = hft::core::EventType::MarketData;
    
    auto start = std::chrono::steady_clock::now();
    const int iterations = 1000000;
    
    for (int i = 0; i < iterations; ++i) {
        queue.push(ev);
        queue.pop();
    }
    
    auto end = std::chrono::steady_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    
    std::cout << "EventQueue Push/Pop: " << iterations << " iterations in " 
              << diff / 1e6 << " ms (" << diff / iterations << " ns/op)" << std::endl;
}

int main() {
    std::cout << "Starting Benchmarks..." << std::endl;
    benchmark_order_book();
    benchmark_event_queue();
    return 0;
}
