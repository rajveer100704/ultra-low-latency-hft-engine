#include "trade_parser.hpp"
#include <spdlog/spdlog.h>

namespace hft::parser {

TradeParser::TradeParser() {}

std::optional<hft::core::MarketDataEvent> TradeParser::parse(const std::string& json_str) {
    try {
        simdjson::padded_string padded(json_str);
        auto doc = parser_.iterate(padded);

        hft::core::MarketDataEvent event;
        
        // Binance @trade format:
        // {
        //   "e": "trade",     // Event type
        //   "E": 123456789,   // Event time
        //   "s": "BNBBTC",    // Symbol
        //   "t": 12345,       // Trade ID
        //   "p": "0.001",     // Price
        //   "q": "100",       // Quantity
        //   "b": 88,          // Buyer order ID
        //   "a": 50,          // Seller order ID
        //   "T": 123456785,   // Trade time
        //   "m": true,        // Is the buyer the market maker?
        //   "M": true         // Ignore
        // }

        std::string_view symbol_view = doc["s"];
        size_t len = std::min(symbol_view.length(), (size_t)15);
        std::copy_n(symbol_view.data(), len, event.symbol);
        event.symbol[len] = '\0';
        
        std::string_view price_str = doc["p"];
        event.price = std::stod(std::string(price_str));
        
        std::string_view qty_str = doc["q"];
        event.quantity = std::stod(std::string(qty_str));
        
        event.exchange_ts = doc["T"];
        event.is_buyer_maker = doc["m"];

        return event;
    } catch (const std::exception& e) {
        spdlog::error("Parse Error: {}", e.what());
        return std::nullopt;
    }
}

} // namespace hft::parser
