#include <iostream>
#include "DBConnection.h"
#include "OrderRepository.h"
#include "TradeRepository.h"

int main() {
    try {
        // ─── Connect ─────────────────────────────────────────────────────
        DBConnection db;
        db.connect();

        OrderRepository orderRepo(db);
        TradeRepository tradeRepo(db);

        // ─── Fetch markets ────────────────────────────────────────────────
        pqxx::work txn(db.get());
        pqxx::result markets = txn.exec("SELECT market_id, symbol, name FROM markets");
        txn.commit();

        std::cout << "\n=== Markets ===\n";
        if (markets.empty()) {
            std::cout << "  (none)\n";
        }
        for (const auto& row : markets) {
            std::cout << "  [" << row["market_id"].as<int>() << "] "
                      << row["symbol"].as<std::string>() << " — "
                      << row["name"].as<std::string>() << "\n";
        }

        // ─── Fetch orders ─────────────────────────────────────────────────
        std::cout << "\n=== Open Orders (market 1) ===\n";
        auto orders = orderRepo.getOpenOrdersByMarket(1);
        if (orders.empty()) {
            std::cout << "  (none)\n";
        }
        for (const auto& o : orders) {
            std::cout << "  OrderID=" << o.orderId
                      << " UserID="  << o.userId
                      << " "         << o.side
                      << " "         << o.symbol
                      << " qty="     << o.quantity
                      << " price="   << o.price
                      << " status="  << o.status << "\n";
        }

        // ─── Fetch trades ─────────────────────────────────────────────────
        std::cout << "\n=== Recent Trades (market 1) ===\n";
        auto trades = tradeRepo.getTradesByMarket(1, 10);
        if (trades.empty()) {
            std::cout << "  (none)\n";
        }
        for (const auto& t : trades) {
            std::cout << "  TradeID="  << t.tradeId
                      << " symbol="    << t.symbol
                      << " price="     << t.execPrice
                      << " qty="       << t.quantity
                      << " buyOrder="  << t.buyOrderId
                      << " sellOrder=" << t.sellOrderId << "\n";
        }

        std::cout << "\n✓ DB connection OK\n\n";

    } catch (const std::exception& e) {
        std::cerr << "\n✗ Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}