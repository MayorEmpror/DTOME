#include "OrderRepository.h"
#include <stdexcept>
#include <iostream>

OrderRepository::OrderRepository(DBConnection& db)
    : m_db(db)
{}

int OrderRepository::insertOrder(const OrderRecord& rec) {
    try {
        pqxx::work txn(m_db.get());

        pqxx::result r = txn.exec_params(
            "INSERT INTO orders "
            "(user_id, market_id, symbol, side, order_type, price, "
            " quantity, remaining_qty, status, timestamp) "
            "VALUES ($1,$2,$3,$4,$5,$6,$7,$8,$9,$10) "
            "RETURNING order_id",
            rec.userId,
            rec.marketId,
            rec.symbol,
            rec.side,
            rec.orderType,
            rec.price,
            rec.quantity,
            rec.remainingQty,
            rec.status,
            rec.timestamp
        );

        txn.commit();
        return r[0][0].as<int>();

    } catch (const std::exception& e) {
        throw std::runtime_error(
            std::string("[OrderRepository::insertOrder] ") + e.what()
        );
    }
}

bool OrderRepository::updateOrderStatus(int orderId,
                                         const std::string& status,
                                         int remainingQty) {
    try {
        pqxx::work txn(m_db.get());

        pqxx::result r = txn.exec_params(
            "UPDATE orders SET status = $1, remaining_qty = $2 "
            "WHERE order_id = $3",
            status,
            remainingQty,
            orderId
        );

        txn.commit();
        return r.affected_rows() > 0;

    } catch (const std::exception& e) {
        throw std::runtime_error(
            std::string("[OrderRepository::updateOrderStatus] ") + e.what()
        );
    }
}

std::optional<OrderRecord> OrderRepository::getOrderById(int orderId) {
    try {
        pqxx::work txn(m_db.get());

        pqxx::result r = txn.exec_params(
            "SELECT order_id, user_id, market_id, symbol, side, order_type, "
            "       price, quantity, remaining_qty, status, timestamp "
            "FROM orders WHERE order_id = $1",
            orderId
        );

        txn.commit();

        if (r.empty()) return std::nullopt;
        return rowToRecord(r[0]);

    } catch (const std::exception& e) {
        throw std::runtime_error(
            std::string("[OrderRepository::getOrderById] ") + e.what()
        );
    }
}

std::vector<OrderRecord> OrderRepository::getOpenOrdersByMarket(int marketId) {
    try {
        pqxx::work txn(m_db.get());

        pqxx::result r = txn.exec_params(
            "SELECT order_id, user_id, market_id, symbol, side, order_type, "
            "       price, quantity, remaining_qty, status, timestamp "
            "FROM orders "
            "WHERE market_id = $1 AND status IN ('OPEN','PARTIALLY_FILLED') "
            "ORDER BY timestamp ASC",
            marketId
        );

        txn.commit();

        std::vector<OrderRecord> records;
        records.reserve(static_cast<size_t>(r.size()));
        for (const auto& row : r)
            records.push_back(rowToRecord(row));
        return records;

    } catch (const std::exception& e) {
        throw std::runtime_error(
            std::string("[OrderRepository::getOpenOrdersByMarket] ") + e.what()
        );
    }
}

std::vector<OrderRecord> OrderRepository::getOrdersByUser(int userId, int limit) {
    try {
        pqxx::work txn(m_db.get());

        pqxx::result r = txn.exec_params(
            "SELECT order_id, user_id, market_id, symbol, side, order_type, "
            "       price, quantity, remaining_qty, status, timestamp "
            "FROM orders "
            "WHERE user_id = $1 "
            "ORDER BY timestamp DESC LIMIT $2",
            userId,
            limit
        );

        txn.commit();

        std::vector<OrderRecord> records;
        records.reserve(static_cast<size_t>(r.size()));
        for (const auto& row : r)
            records.push_back(rowToRecord(row));
        return records;

    } catch (const std::exception& e) {
        throw std::runtime_error(
            std::string("[OrderRepository::getOrdersByUser] ") + e.what()
        );
    }
}

// ─── private ──────────────────────────────────────────────────────────────────

OrderRecord OrderRepository::rowToRecord(const pqxx::row& row) const {
    OrderRecord rec;
    rec.orderId      = row["order_id"].as<int>();
    rec.userId       = row["user_id"].as<int>();
    rec.marketId     = row["market_id"].as<int>();
    rec.symbol       = row["symbol"].as<std::string>();
    rec.side         = row["side"].as<std::string>();
    rec.orderType    = row["order_type"].as<std::string>();
    rec.price        = row["price"].as<double>();
    rec.quantity     = row["quantity"].as<int>();
    rec.remainingQty = row["remaining_qty"].as<int>();
    rec.status       = row["status"].as<std::string>();
    rec.timestamp    = row["timestamp"].as<long>();
    return rec;
}