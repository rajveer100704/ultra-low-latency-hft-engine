#include "order_book.hpp"
#include <algorithm>
#include <mutex>
#include <atomic>

namespace hft::orderbook {

void OrderBook::apply_trade(const hft::core::MarketDataEvent& trade) {
    std::lock_guard<std::mutex> lock(mutex_);
    last_price_ = trade.price;
    
    if (trade.is_buyer_maker) {
        sell_volume_.fetch_add(trade.quantity, std::memory_order_relaxed);
        // Update bids (simplified: find or replace)
        bool found = false;
        for (int i = 0; i < num_bids_; ++i) {
            if (bid_levels_[i].price == trade.price) {
                bid_levels_[i].quantity = trade.quantity;
                found = true;
                break;
            }
        }
        if (!found && num_bids_ < MaxLevels) {
            bid_levels_[num_bids_++] = {trade.price, trade.quantity};
        }
        std::sort(bid_levels_, bid_levels_ + num_bids_, [](const auto& a, const auto& b) {
            return a.price > b.price;
        });
    } else {
        buy_volume_.fetch_add(trade.quantity, std::memory_order_relaxed);
        // Update asks
        bool found = false;
        for (int i = 0; i < num_asks_; ++i) {
            if (ask_levels_[i].price == trade.price) {
                ask_levels_[i].quantity = trade.quantity;
                found = true;
                break;
            }
        }
        if (!found && num_asks_ < MaxLevels) {
            ask_levels_[num_asks_++] = {trade.price, trade.quantity};
        }
        std::sort(ask_levels_, ask_levels_ + num_asks_, [](const auto& a, const auto& b) {
            return a.price < b.price;
        });
    }

    // Recalculate imbalance and spread
    if (num_bids_ > 0 && num_asks_ > 0) {
        double best_bid = bid_levels_[0].price;
        double best_ask = ask_levels_[0].price;
        double bid_sz = bid_levels_[0].quantity;
        double ask_sz = ask_levels_[0].quantity;
        
        imbalance_.store((bid_sz - ask_sz) / (bid_sz + ask_sz), std::memory_order_relaxed);
        spread_.store(best_ask - best_bid, std::memory_order_relaxed);
    }
}

std::vector<Level> OrderBook::get_top_bids(int n) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Level> result;
    int count = std::min(n, num_bids_);
    for (int i = 0; i < count; ++i) {
        result.push_back(bid_levels_[i]);
    }
    return result;
}

std::vector<Level> OrderBook::get_top_asks(int n) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Level> result;
    int count = std::min(n, num_asks_);
    for (int i = 0; i < count; ++i) {
        result.push_back(ask_levels_[i]);
    }
    return result;
}

} // namespace hft::orderbook
