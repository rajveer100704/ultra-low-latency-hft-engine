#include "core/event_queue.hpp"
#include "feed/imarket_feed.hpp"
#include "feed/hybrid_feed.hpp"
#include "feed/websocket_client.hpp"
#include "replay/replay_engine.hpp"
#include "replay/market_data_recorder.hpp"
#include "analytics/backtest_metrics.hpp"
#include "analytics/latency_monitor.hpp"
#include "parser/trade_parser.hpp"
#include "orderbook/order_book.hpp"
#include "strategy/strategy_engine.hpp"
#include "strategy/market_maker_strategy.hpp"
#include "strategy/momentum_strategy.hpp"
#include "risk/risk_engine.hpp"
#include "pnl/pnl_engine.hpp"
#include "ui/terminal_ui.hpp"
#include "oms/order_manager.hpp"
#include "exchange/matching_engine.hpp"
#include "core/thread_utils.hpp"
#include "core/timer_utils.hpp"
#include <thread>
#include <atomic>
#include <spdlog/spdlog.h>
#include <cstdint>
#include <vector>
#include <mutex>
#include <chrono>
#include <algorithm>
#include <memory>
#include <string>
#include <optional>
#include <array>

int main(int argc, char* argv[]) {
    spdlog::set_level(spdlog::level::info);
    
    enum class Mode { LIVE, REPLAY, HYBRID };
    Mode mode = Mode::LIVE;
    std::string data_file = "market_data_record.csv";
    double replay_speed = 1.0;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--replay" && i + 1 < argc) {
            mode = Mode::REPLAY;
            data_file = argv[++i];
        } else if (arg == "--live") {
            mode = Mode::LIVE;
        } else if (arg == "--hybrid") {
            mode = Mode::HYBRID;
        } else if (arg == "--speed" && i + 1 < argc) {
            replay_speed = std::stod(argv[++i]);
        }
    }

    spdlog::info("Starting HFT system | Mode: {} | Speed: {}x", 
                 (mode == Mode::LIVE ? "LIVE" : (mode == Mode::REPLAY ? "REPLAY" : "HYBRID")), 
                 (mode == Mode::LIVE ? 1.0 : replay_speed));

    hft::core::EventQueue event_queue;
    hft::orderbook::OrderBook order_book;
    
    std::unique_ptr<hft::feed::IMarketFeed> market_feed;
    std::unique_ptr<hft::replay::MarketDataRecorder> recorder;
    
    if (mode == Mode::LIVE) {
        auto ws = std::make_unique<hft::feed::WebSocketClient>(event_queue);
        ws->connect("stream.binance.com", "9443", "/ws/btcusdt@trade");
        market_feed = std::move(ws);
        recorder = std::make_unique<hft::replay::MarketDataRecorder>(data_file);
    } else if (mode == Mode::REPLAY) {
        market_feed = std::make_unique<hft::replay::ReplayEngine>(event_queue, data_file, replay_speed);
    } else if (mode == Mode::HYBRID) {
        auto hybrid = std::make_unique<hft::feed::HybridFeed>();
        
        auto ws = std::make_unique<hft::feed::WebSocketClient>(event_queue);
        ws->connect("stream.binance.com", "9443", "/ws/btcusdt@trade");
        hybrid->add_feed(std::move(ws));
        
        hybrid->add_feed(std::make_unique<hft::replay::ReplayEngine>(event_queue, data_file, replay_speed));
        
        market_feed = std::move(hybrid);
        recorder = std::make_unique<hft::replay::MarketDataRecorder>("hybrid_record.csv");
    }
    
    hft::parser::TradeParser parser;
    hft::strategy::StrategyEngine strategy(event_queue, order_book);
    
    // Add strategies using static dispatch (variant)
    strategy.add_strategy(hft::strategy::MarketMakerStrategy(event_queue, order_book));
    strategy.add_strategy(hft::strategy::MomentumStrategy(event_queue, order_book));

    hft::risk::RiskEngine risk(100.0); // 100 BTC max pos
    hft::pnl::PnLEngine pnl_engine;
    hft::ui::TerminalUI ui(order_book);

    hft::oms::OrderManager order_manager(event_queue, risk);
    hft::exchange::MatchingEngine matching_engine(event_queue);
    hft::analytics::LatencyMonitor latency_monitor;
    std::atomic<bool> running{true};
    uint64_t last_pushed = 0;
    auto last_telemetry_time = std::chrono::steady_clock::now();
    std::vector<hft::core::ExecutionReport> trade_history;
    std::mutex trade_mutex;

    // Feed Thread
    std::thread feed_thread([&]() {
        hft::core::ThreadUtils::pin_to_core(1); // Pin feed to core 1
        if (market_feed) {
            market_feed->run();
        }
    });

    // Processor Thread (Event Loop)
    std::thread processor_thread([&]() {
        hft::core::ThreadUtils::pin_to_core(2); // Pin processor to core 2
        std::array<std::optional<hft::core::Event>, 4> batch;
        while (running) {
            int found = 0;
            for (int i = 0; i < 4; ++i) {
                batch[i] = event_queue.pop();
                if (batch[i]) found++;
                else break;
            }

            if (found == 0) {
                // Busy Poll: No yield for sub-100us performance
                continue;
            }

            for (int i = 0; i < found; ++i) {
                if (!batch[i].has_value()) continue;
                hft::core::Event& event = batch[i].value();

                switch (event.type) {
                    case hft::core::EventType::MarketData:
                        event.strategy_ts = hft::core::TimerUtils::rdtsc(); 
                        strategy.update_position(pnl_engine.get_total_position());
                        strategy.on_market_data(event.market_data);
                        matching_engine.update_market_price(event.market_data);
                        break;

                    case hft::core::EventType::OrderRequest:
                        order_manager.handle_order_request(event.order_request);
                        break;
                        
                    case hft::core::EventType::ValidatedOrder:
                        matching_engine.process_order(event.order_request);
                        break;
                        
                    case hft::core::EventType::CancelOrder:
                        matching_engine.cancel_order(event.cancel_order.order_id);
                        break;

                    case hft::core::EventType::ExecutionReport: {
                        auto& report = event.execution_report;
                        order_manager.handle_execution_report(report);
                        strategy.on_execution(report);
                        
                        if (report.status == hft::core::OrderStatus::Filled) {
                            pnl_engine.on_fill(report.strategy_id, report.side, report.last_price, report.last_qty);
                            double side_sign = (report.side == hft::core::OrderSide::Buy) ? 1.0 : -1.0;
                            risk.update_position(report.last_qty * side_sign);

                            latency_monitor.record_signal_to_order(report.signal_ts, report.order_ts);
                            latency_monitor.record_order_to_fill(report.order_ts, hft::core::LatencyTracker::now_ns());
                            latency_monitor.record_total_latency(report.signal_ts, hft::core::LatencyTracker::now_ns());
                        }
                        pnl_engine.update_pnl(report.last_price); 
                        
                        if (report.status == hft::core::OrderStatus::Filled) {
                            std::lock_guard<std::mutex> lock(trade_mutex);
                            trade_history.push_back(report);
                            if (trade_history.size() > 1000) {
                                trade_history.erase(trade_history.begin());
                            }
                        }
                        break;
                    }
                    default: break;
                }
            }
        }
    });

    // UI Loop
    int ui_counter = 0;
    while (!ui.should_close() && running) {
        if (mode == Mode::REPLAY && market_feed && market_feed->is_finished()) {
            spdlog::info("Backtest complete.");
            running = false;
            break;
        }
        
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - last_telemetry_time).count();
        uint64_t current_pushed = event_queue.total_pushed();
        uint64_t ev_per_sec = (duration > 0) ? (current_pushed - last_pushed) / duration : 0;
        
        std::vector<hft::core::ExecutionReport> trades_to_show;
        {
            std::lock_guard<std::mutex> lock(trade_mutex);
            trades_to_show = trade_history;
        }

        // Get consolidated latency stats
        auto total_latency_stats = latency_monitor.get_total_latency_stats();
        
        // Final Summary to Terminal (or Dashboard)
        if (ui_counter % 2 == 0) { // Assuming ui_counter is incremented elsewhere or this is a simplified example
            ui.update(pnl_engine.get_total_pnl(), 
                      pnl_engine.get_total_position(),
                      pnl_engine.get_strategy_stats(),
                      total_latency_stats.p99_ns, 
                      event_queue.size(),
                      ev_per_sec,
                      order_manager.get_active_orders(),
                      trades_to_show);
        }
        
        if (duration >= 1) {
            last_pushed = current_pushed;
            last_telemetry_time = now;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Fills render queue slightly faster for replay
        ui_counter++;
    }

    running = false;
    if (market_feed) market_feed->stop();
    
    if (feed_thread.joinable()) feed_thread.join();
    if (processor_thread.joinable()) processor_thread.join();

    spdlog::info("HFT system shut down.");
    
    // Final Performance Metrics
    hft::analytics::BacktestMetrics metrics;
    metrics.total_pnl = pnl_engine.get_total_pnl();
    metrics.max_drawdown = pnl_engine.get_max_drawdown();
    metrics.trade_count = pnl_engine.get_total_trades();
    metrics.win_rate = pnl_engine.get_win_rate() * 100.0;

    spdlog::info("=== FINAL PERFORMANCE SUMMARY ===");
    spdlog::info("Total PnL:      {:.2f}", metrics.total_pnl);
    spdlog::info("Max Drawdown:   {:.2f}", metrics.max_drawdown);
    spdlog::info("Total Trades:   {}", metrics.trade_count);
    spdlog::info("Win Rate:       {:.2f}%", metrics.win_rate);
    spdlog::info("=================================");
    
    return 0;
}
