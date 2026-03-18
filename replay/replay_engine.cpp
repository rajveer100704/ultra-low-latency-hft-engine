#include "replay_engine.hpp"
#include <spdlog/spdlog.h>
#include <sstream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cstring>
#include <string>

namespace hft::replay {

ReplayEngine::ReplayEngine(hft::core::EventQueue& event_queue, const std::string& filename, double speed)
    : event_queue_(event_queue), filename_(filename), speed_(speed) {
    file_.open(filename_);
    if (!file_.is_open()) {
        spdlog::error("ReplayEngine: Could not open file {}", filename_);
    }
}

ReplayEngine::~ReplayEngine() {
    stop();
}

void ReplayEngine::run() {
    running_ = true;
    finished_ = false;

    if (!file_.is_open()) {
        finished_ = true;
        return;
    }

    std::string line;
    uint64_t last_event_ts = 0;
    
    while (running_ && std::getline(file_, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string token;
        
        hft::core::Event event;
        event.type = hft::core::EventType::MarketData;
        auto& md = event.market_data;

        // format: symbol,price,quantity,exchange_ts,is_buyer_maker
        if (!std::getline(ss, token, ',')) continue;
        std::strncpy(md.symbol, token.c_str(), 15);
        md.symbol[15] = '\0';
        
        if (std::getline(ss, token, ',')) md.price = std::stod(token);
        if (std::getline(ss, token, ',')) md.quantity = std::stod(token);
        if (std::getline(ss, token, ',')) md.exchange_ts = std::stoll(token);
        if (std::getline(ss, token, ',')) md.is_buyer_maker = std::stoi(token);

        // Precise Timing Logic
        if (last_event_ts > 0 && speed_ > 0) {
            uint64_t delta_ns = (md.exchange_ts > last_event_ts) ? (md.exchange_ts - last_event_ts) : 0;
            if (delta_ns > 0) {
                auto sleep_duration = std::chrono::nanoseconds(static_cast<long long>(delta_ns / speed_));
                // Cap sleep to avoid stalls if data has huge gaps
                if (sleep_duration > std::chrono::seconds(1)) sleep_duration = std::chrono::milliseconds(10);
                std::this_thread::sleep_for(sleep_duration);
            }
        }
        
        last_event_ts = md.exchange_ts;
        event.receive_ts = std::chrono::steady_clock::now().time_since_epoch().count();
        event_queue_.push(event);
    }

    running_ = false;
    finished_ = true;
    spdlog::info("ReplayEngine: Finished replaying {}", filename_);
}

void ReplayEngine::stop() {
    running_ = false;
}

} // namespace hft::replay
