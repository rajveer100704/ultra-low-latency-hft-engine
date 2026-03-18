#pragma once

#include <string>
#include <fstream>
#include "core/events.hpp"

namespace hft::replay {

class MarketDataRecorder {
public:
    MarketDataRecorder(const std::string& filename);
    ~MarketDataRecorder();

    void record(const hft::core::MarketDataEvent& md);
    bool is_open() const { return file_.is_open(); }

private:
    std::ofstream file_;
};

} // namespace hft::replay
