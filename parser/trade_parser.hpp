#pragma once

#include <string>
#include <optional>
#include "core/events.hpp"
#include <simdjson.h>

namespace hft::parser {

class TradeParser {
public:
    TradeParser();
    std::optional<hft::core::MarketDataEvent> parse(const std::string& json_str);

private:
    simdjson::ondemand::parser parser_;
};

} // namespace hft::parser
