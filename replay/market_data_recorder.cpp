#include "market_data_recorder.hpp"
#include <spdlog/spdlog.h>
#include <iostream>
#include <fstream>
#include <string>

namespace hft::replay {

MarketDataRecorder::MarketDataRecorder(const std::string& filename) {
    file_.open(filename, std::ios::app);
    if (!file_.is_open()) {
        spdlog::error("MarketDataRecorder: Could not open {} for recording", filename);
    }
}

MarketDataRecorder::~MarketDataRecorder() {
    if (file_.is_open()) {
        file_.close();
    }
}

void MarketDataRecorder::record(const hft::core::MarketDataEvent& md) {
    if (file_.is_open()) {
        file_ << md.symbol << "," 
              << md.price << "," 
              << md.quantity << "," 
              << md.exchange_ts << "," 
              << md.is_buyer_maker << "\n";
        file_.flush(); // Flush often for safety during crashes/forced kills
    }
}

} // namespace hft::replay
