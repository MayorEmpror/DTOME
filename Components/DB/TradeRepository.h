#pragma once

#include "DBConnection.h"
#include <string>
#include <vector>
#include <optional>

// ─── Mirrors the trades table row ─────────────────────────────────────────────
struct TradeRecord {
    long        tradeId;
    int         marketId;
    int         buyOrderId;
    int         sellOrderId;
    std::string symbol;
    double      execPrice;
    int         quantity;
    long        timestamp;
};
    
// ─── TradeRepository ──────────────────────────────────────────────────────────
//  All DB operations for the trades table.
//  insertTrade() is the only write path — trades are immutable once created.
//
class TradeRepository {
public:
    explicit TradeRepository(DBConnection& db);

    // Insert a completed trade — returns DB-assigned tradeId
    long insertTrade(const TradeRecord& rec);

    // Fetch recent trades for a market (order book / price feed display)
    std::vector<TradeRecord> getTradesByMarket(int marketId, int limit = 50);

    // Fetch all trades a user was a party to (buy or sell side)
    std::vector<TradeRecord> getTradesByUser(int userId, int limit = 100);

    // Fetch a single trade by ID
    std::optional<TradeRecord> getTradeById(long tradeId);

private:
    DBConnection& m_db;
    TradeRecord   rowToRecord(const pqxx::row& row) const;
};