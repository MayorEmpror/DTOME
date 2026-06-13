#pragma once

#include "DBConnection.h"
#include <string>
#include <vector>
#include <optional>

// ─── Mirrors the orders table row ─────────────────────────────────────────────
struct OrderRecord {
    int         orderId;
    int         userId;
    int         marketId;
    std::string symbol;
    std::string side;        // "BUY" | "SELL"
    std::string orderType;   // "LIMIT" | "MARKET"
    double      price;
    int         quantity;
    int         remainingQty;
    std::string status;      // "OPEN" | "FILLED" | "PARTIALLY_FILLED" | "CANCELLED"
    long        timestamp;
};

// ─── OrderRepository ──────────────────────────────────────────────────────────
//  All DB operations for the orders table.
//  Every method opens its own transaction — keep calls off the hot matching path.
//
class OrderRepository {
public:
    explicit OrderRepository(DBConnection& db);

    // Insert a new order row — returns the DB-assigned orderId
    int  insertOrder(const OrderRecord& rec);

    // Update status and remainingQty after a fill or cancel
    bool updateOrderStatus(int orderId, const std::string& status, int remainingQty);

    // Fetch a single order by ID
    std::optional<OrderRecord> getOrderById(int orderId);

    // Fetch all open orders for a given market (used on engine startup to reload state)
    std::vector<OrderRecord> getOpenOrdersByMarket(int marketId);

    // Fetch all orders for a user (for UI display)
    std::vector<OrderRecord> getOrdersByUser(int userId, int limit = 100);

private:
    DBConnection& m_db;
    OrderRecord   rowToRecord(const pqxx::row& row) const;
};