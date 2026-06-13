#pragma once

#include <pqxx/pqxx>
#include <string>
#include <memory>
#include <stdexcept>

// ─── DBConnection ─────────────────────────────────────────────────────────────
//  Owns a single pqxx::connection to PostgreSQL.
//  All repository classes hold a reference to this — one connection
//  is shared across the app for now (sufficient for a single-server process).
//
//  Usage:
//      DBConnection db;          // reads DTOME_DB_URL from environment
//      db.connect();
//      auto& conn = db.get();    // pass to repositories
//
class DBConnection {
public:
    DBConnection();
    ~DBConnection() = default;

    // No copy — one connection per process
    DBConnection(const DBConnection&)            = delete;
    DBConnection& operator=(const DBConnection&) = delete;

    // Connect using DTOME_DB_URL environment variable
    void connect();

    // Returns true if connection is open and healthy
    bool isConnected() const;

    // Attempt reconnect (call if isConnected() returns false)
    void reconnect();

    // Raw pqxx connection — passed to repositories
    pqxx::connection& get();

private:
    std::string                    m_url;
    std::unique_ptr<pqxx::connection> m_conn;

    std::string loadUrlFromEnv() const;
};