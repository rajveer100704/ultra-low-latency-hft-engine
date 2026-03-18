#pragma once

#include "imarket_feed.hpp"
#include <vector>
#include <memory>
#include <thread>
#include <atomic>

namespace hft::feed {

/**
 * @brief HybridFeed multiplexes multiple IMarketFeed sources into the system.
 * This allows running Live and Replay data streams concurrently.
 */
class HybridFeed : public IMarketFeed {
public:
    HybridFeed() = default;

    void add_feed(std::unique_ptr<IMarketFeed> feed) {
        feeds_.push_back(std::move(feed));
    }

    void run() override {
        running_ = true;
        std::vector<std::thread> threads;
        
        for (auto& feed : feeds_) {
            threads.emplace_back([&]() {
                feed->run();
            });
        }

        for (auto& t : threads) {
            if (t.joinable()) t.join();
        }
    }

    void stop() override {
        running_ = false;
        for (auto& feed : feeds_) {
            feed->stop();
        }
    }

    bool is_active() const override {
        for (auto& feed : feeds_) {
            if (feed->is_active()) return true;
        }
        return false;
    }

    bool is_finished() const override {
        for (auto& feed : feeds_) {
            if (!feed->is_finished()) return false;
        }
        return true;
    }

private:
    std::vector<std::unique_ptr<IMarketFeed>> feeds_;
    std::atomic<bool> running_{false};
};

} // namespace hft::feed
