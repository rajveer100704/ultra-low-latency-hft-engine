#pragma once

#include <string>
#include <fstream>
#include <atomic>
#include "core/event_queue.hpp"

#include "feed/imarket_feed.hpp"

namespace hft::replay {

class ReplayEngine : public hft::feed::IMarketFeed {
public:
    ReplayEngine(hft::core::EventQueue& event_queue, const std::string& filename, double speed = 1.0);
    ~ReplayEngine();

    void run() override;
    void stop() override;
    bool is_active() const override { return !finished_; }
    bool is_finished() const override { return finished_; }

private:
    hft::core::EventQueue& event_queue_;
    std::string filename_;
    double speed_;
    std::atomic<bool> running_{false};
    std::atomic<bool> finished_{false};
    std::ifstream file_;
};

} // namespace hft::replay
