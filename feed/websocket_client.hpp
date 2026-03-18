#pragma once

#include <string>
#include <functional>
#include <memory>
#include "core/event_queue.hpp"

#include "imarket_feed.hpp"

namespace hft::feed {

class WebSocketClient : public IMarketFeed {
public:
    WebSocketClient(hft::core::EventQueue& event_queue);
    ~WebSocketClient();

    void connect(const std::string& host, const std::string& port, const std::string& target);
    void run() override;
    void stop() override;
    bool is_active() const override { return true; } // Live is always active until stopped

private:
    hft::core::EventQueue& event_queue_;
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace hft::feed
