#pragma once

namespace hft::feed {

/**
 * @brief Common interface for all market data sources (Live, Replay, Hybrid).
 */
class IMarketFeed {
public:
    virtual ~IMarketFeed() = default;

    // Start fetching and pushing market data to the event queue
    virtual void run() = 0;

    // Gracefully stop the feed
    virtual void stop() = 0;

    // Optional: check if the feed is still active (e.g., for replay)
    virtual bool is_active() const = 0;
    virtual bool is_finished() const { return false; }
};

} // namespace hft::feed
