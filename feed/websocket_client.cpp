#include "websocket_client.hpp"
#include <spdlog/spdlog.h>
#include <chrono>
#include <thread>
#include <random>
#include <algorithm>
#include <cstring>

#ifdef USE_BOOST
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;
#endif

namespace hft::feed {

struct WebSocketClient::Impl {
    hft::core::EventQueue& event_queue;
    bool running = false;
#ifdef USE_BOOST
    net::io_context ioc;
    ssl::context ctx{ssl::context::tlsv12_client};
    std::unique_ptr<websocket::stream<beast::ssl_stream<beast::tcp_stream>>> ws;
#endif

    Impl(hft::core::EventQueue& eq) : event_queue(eq) {}
};

WebSocketClient::WebSocketClient(hft::core::EventQueue& event_queue)
    : event_queue_(event_queue), impl_(std::make_unique<Impl>(event_queue)) {
#ifdef USE_BOOST
    impl_->ctx.set_default_verify_paths();
    impl_->ctx.set_verify_mode(ssl::verify_none);
#endif
}

WebSocketClient::~WebSocketClient() {
    stop();
}

void WebSocketClient::connect(const std::string& host, const std::string& port, const std::string& target) {
#ifdef USE_BOOST
    try {
        tcp::resolver resolver(impl_->ioc);
        impl_->ws = std::make_unique<websocket::stream<beast::ssl_stream<beast::tcp_stream>>>(impl_->ioc, impl_->ctx);
        auto const results = resolver.resolve(host, port);
        beast::get_lowest_layer(*impl_->ws).connect(results);
        impl_->ws->next_layer().handshake(ssl::stream_base::client);
        impl_->ws->set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req) {
                req.set(http::field::user_agent, "hft-client");
            }));
        impl_->ws->handshake(host, target);
        spdlog::info("Connected to WebSocket at {}:{}{}", host, port, target);
        impl_->running = true;
    } catch (const std::exception& e) {
        spdlog::error("WebSocket Connection Error: {}", e.what());
    }
#else
    spdlog::warn("Boost not found. Using SIMULATED market data feed.");
    impl_->running = true;
#endif
}

void WebSocketClient::run() {
#ifdef USE_BOOST
    beast::flat_buffer buffer;
    while (impl_->running) {
        try {
            impl_->ws->read(buffer);
            uint64_t receive_ts = std::chrono::steady_clock::now().time_since_epoch().count();
            std::string message = beast::buffers_to_string(buffer.data());
            buffer.consume(buffer.size());
            
            // In the original system, we might push the raw message.
            // For this demo, we'll log it.
            spdlog::debug("Received data: {}", message);
        } catch (...) { impl_->running = false; }
    }
#else
    std::mt19937 gen(42);
    std::uniform_real_distribution<> dis(-0.5, 0.5);
    double price = 50000.0;

    while (impl_->running) {
        price += dis(gen);
        hft::core::Event event;
        event.type = hft::core::EventType::MarketData;
        event.receive_ts = std::chrono::steady_clock::now().time_since_epoch().count();
        
        auto& md = event.market_data;
        const char* symbol = "BTCUSDT";
        std::copy_n(symbol, 7, md.symbol);
        md.symbol[7] = '\0';
        md.price = price;
        md.quantity = 0.01;
        md.exchange_ts = event.receive_ts - 1000000; // 1ms lag
        md.is_buyer_maker = (dis(gen) > 0);

        event_queue_.push(event);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
#endif
}

void WebSocketClient::stop() {
    impl_->running = false;
#ifdef USE_BOOST
    if (impl_->ws) {
        try { impl_->ws->close(websocket::close_code::normal); } catch (...) {}
    }
#endif
}

} // namespace hft::feed
