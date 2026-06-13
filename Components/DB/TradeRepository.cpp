#include "TradeRepository.h"
#include <stdexcept>

TradeRepository::TradeRepository(DBConnection& db)
    : m_db(db)
{}

long TradeRepository::insertTrade(const TradeRecord& rec) {
    try {
        pqxx::work txn(m_db.get());

        pqxx::result r = txn.exec_params(
            "INSERT INTO trades "
            "(market_id, buy_order_id, sell_order_id, symbol, "
            " exec_price, quantity, timestamp) "
            "VALUES ($1,$2,$3,$4,$5,$6,$7) "
            "RETURNING trade_id",
            rec.marketId,
            rec.buyOrderId,
            rec.sellOrderId,
            rec.symbol,
            rec.execPrice,
            rec.quantity,
            rec.timestamp
        );

        txn.commit();
        return r[0][0].as<long>();

    } catch (const std::exception& e) {
        throw std::runtime_error(
            std::string("[TradeRepository::insertTrade] ") + e.what()
        );
    }
}

std::vector<TradeRecord> TradeRepository::getTradesByMarket(int marketId, int limit) {
    try {
        pqxx::work txn(m_db.get());

        pqxx::result r = txn.exec_params(
            "SELECT trade_id, market_id, buy_order_id, sell_order_id, "
            "       symbol, exec_price, quantity, timestamp "
            "FROM trades "
            "WHERE market_id = $1 "
            "ORDER BY timestamp DESC LIMIT $2",
            marketId,
            limit
        );

        txn.commit();

        std::vector<TradeRecord> records;
        records.reserve(static_cast<size_t>(r.size()));
        for (const auto& row : r)
            records.push_back(rowToRecord(row));
        return records;

    } catch (const std::exception& e) {
        throw std::runtime_error(
            std::string("[TradeRepository::getTradesByMarket] ") + e.what()
        );
    }
}

std::vector<TradeRecord> TradeRepository::getTradesByUser(int userId, int limit) {
    try {
        pqxx::work txn(m_db.get());

        // Join against orders to find trades where user was on either side
        pqxx::result r = txn.exec_params(
            "SELECT t.trade_id, t.market_id, t.buy_order_id, t.sell_order_id, "
            "       t.symbol, t.exec_price, t.quantity, t.timestamp "
            "FROM trades t "
            "JOIN orders o ON (o.order_id = t.buy_order_id "
            "               OR o.order_id = t.sell_order_id) "
            "WHERE o.user_id = $1 "
            "ORDER BY t.timestamp DESC LIMIT $2",
            userId,
            limit
        );

        txn.commit();

        std::vector<TradeRecord> records;
        records.reserve(static_cast<size_t>(r.size()));
        for (const auto& row : r)
            records.push_back(rowToRecord(row));
        return records;

    } catch (const std::exception& e) {
        throw std::runtime_error(
            std::string("[TradeRepository::getTradesByUser] ") + e.what()
        );
    }
}

std::optional<TradeRecord> TradeRepository::getTradeById(long tradeId) {
    try {
        pqxx::work txn(m_db.get());

        pqxx::result r = txn.exec_params(
            "SELECT trade_id, market_id, buy_order_id, sell_order_id, "
            "       symbol, exec_price, quantity, timestamp "
            "FROM trades WHERE trade_id = $1",
            tradeId
        );

        txn.commit();

        if (r.empty()) return std::nullopt;
        return rowToRecord(r[0]);

    } catch (const std::exception& e) {
        throw std::runtime_error(
            std::string("[TradeRepository::getTradeById] ") + e.what()
        );
    }
}

// ─── private ──────────────────────────────────────────────────────────────────

TradeRecord TradeRepository::rowToRecord(const pqxx::row& row) const {
    TradeRecord rec;
    rec.tradeId     = row["trade_id"].as<long>();
    rec.marketId    = row["market_id"].as<int>();
    rec.buyOrderId  = row["buy_order_id"].as<int>();
    rec.sellOrderId = row["sell_order_id"].as<int>();
    rec.symbol      = row["symbol"].as<std::string>();
    rec.execPrice   = row["exec_price"].as<double>();
    rec.quantity    = row["quantity"].as<int>();
    rec.timestamp   = row["timestamp"].as<long>();
    return rec;
}