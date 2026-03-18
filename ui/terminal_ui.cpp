#include "terminal_ui.hpp"
#ifdef USE_NCURSES
#include <ncurses.h>
#else
// Minimal dummy for when ncurses is missing
inline void initscr() {}
inline void cbreak() {}
inline void noecho() {}
inline void curs_set(int) {}
inline void timeout(int) {}
inline void start_color() {}
inline void init_pair(int, int, int) {}
inline void attron(int) {}
inline void attroff(int) {}
inline void endwin() {}
inline void clear() {}
inline void refresh() {}
inline void mvprintw(int, int, const char*, ...) {}
inline int getch() { return -1; }
#define COLOR_PAIR(X) 0
#define COLOR_GREEN 0
#define COLOR_RED 0
#define COLOR_BLACK 0
#endif
#include <string>

namespace hft::ui {

TerminalUI::TerminalUI(const hft::orderbook::OrderBook& order_book) : order_book_(order_book) {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    timeout(100); // 100ms refresh
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
}

TerminalUI::~TerminalUI() {
    endwin();
}

void TerminalUI::update(double total_pnl, double total_position, 
                        const std::map<uint32_t, hft::pnl::StrategyStats>& strategy_stats,
                        uint64_t p99_latency_ns,
                        size_t queue_depth,
                        uint64_t events_per_sec,
                        const std::vector<hft::core::OrderRequest>& open_orders,
                        const std::vector<hft::core::ExecutionReport>& trades) {
    clear();
    mvprintw(0, 0, "=== HFT QUANT INFRASTRUCTURE - ELITE DASHBOARD ===");
    mvprintw(2, 0, "TOTAL Position: %.4f BTC | TOTAL PnL: %.2f USDT", total_position, total_pnl);

    // Strategy Attribution Table
    mvprintw(4, 0, "--- STRATEGY ATTRIBUTION ---");
    mvprintw(5, 0, "ID   Name         PnL      Pos      Trades");
    int s_row = 6;
    for (const auto& [id, stats] : strategy_stats) {
        std::string name = (id == 1 ? "MarketMaker" : (id == 2 ? "Momentum  " : "Unknown   "));
        mvprintw(s_row++, 0, "%-4u %-12s %-8.2f %-8.4f %-6d", 
                 id, name.c_str(), stats.realized_pnl + stats.unrealized_pnl, stats.position, stats.trades);
    }

    // Left Column: Order Book (Shifted down)
    int row = s_row + 1;
    mvprintw(row++, 0, "--- ORDER BOOK ---");
    mvprintw(row++, 0, "Imbalance: %.2f | Spread: %.2f", order_book_.get_imbalance(), order_book_.get_spread());
    
    auto asks = order_book_.get_top_asks(5);
    auto bids = order_book_.get_top_bids(5);

    attron(COLOR_PAIR(2));
    for (int i = (int)asks.size() - 1; i >= 0; --i) {
        mvprintw(row++, 0, "ASK: %.2f | %.4f", asks[i].price, asks[i].quantity);
    }
    attroff(COLOR_PAIR(2));
    mvprintw(row++, 0, "------------------");
    attron(COLOR_PAIR(1));
    for (const auto& bid : bids) {
        mvprintw(row++, 0, "BID: %.2f | %.4f", bid.price, bid.quantity);
    }
    attroff(COLOR_PAIR(1));

    // Right Column: Orders and Trades
    int col2 = 50;
    mvprintw(4, col2, "--- OPEN ORDERS ---");
    int o_row = 5;
    for (size_t i = 0; i < std::min(open_orders.size(), (size_t)8); ++i) {
        const auto& o = open_orders[i];
        mvprintw(o_row++, col2, "S%u #%llu %s %.4f @ %.2f", 
                 o.strategy_id, o.order_id, (o.side == hft::core::OrderSide::Buy ? "BUY" : "SELL"),
                 o.quantity, o.price);
    }

    mvprintw(o_row + 1, col2, "--- TRADE HISTORY ---");
    int t_row = o_row + 2;
    for (int i = (int)trades.size() - 1; i >= std::max(0, (int)trades.size() - 8); --i) {
        const auto& t = trades[i];
        mvprintw(t_row++, col2, "S%u ID:%llu %s %.4f @ %.2f", 
                 t.strategy_id, t.trade_id, (t.side == hft::core::OrderSide::Buy ? "BOT" : "SLD"),
                 t.last_qty, t.last_price);
    }

    // Telemetry at the bottom
    int b_row = std::max(row, t_row) + 2;
    mvprintw(b_row, 0, "--- SYSTEM TELEMETRY ---");
    mvprintw(b_row + 1, 0, "Throughput:  %-10llu ev/s | Queue Depth: %zu", events_per_sec, queue_depth);
    mvprintw(b_row + 2, 0, "P99 Latency: %-10llu ns   | System Status: NOMINAL", p99_latency_ns);

    mvprintw(b_row + 4, 0, "Press 'q' to quit.");
    
    int ch = getch();
    if (ch == 'q') {
        should_close_ = true;
    }
    refresh();
}

bool TerminalUI::should_close() const {
    return should_close_;
}

} // namespace hft::ui
