#include "DBConnection.h"
#include <cstdlib>
#include <iostream>

DBConnection::DBConnection() : m_url(loadUrlFromEnv()), m_conn(nullptr){}

std::string DBConnection::loadUrlFromEnv() const {
    const char* url = std::getenv("DTOME_DB_URL");
    if (!url || std::string(url).empty()) {
        throw std::runtime_error(
            "[DBConnection] DTOME_DB_URL environment variable not set.\n"
            "  Export it before running:  export DTOME_DB_URL=postgresql://..."
        );
    }
    return std::string(url);
}

void DBConnection::connect() {
    try {
        m_conn = std::make_unique<pqxx::connection>(m_url);
        if (!m_conn->is_open()) {
            throw std::runtime_error("[DBConnection] Connection opened but is_open() returned false.");
        }
        std::cout << "[DBConnection] Connected to PostgreSQL: "
                  << m_conn->dbname() << "\n";
    } catch (const pqxx::broken_connection& e) {
        throw std::runtime_error(
            std::string("[DBConnection] Failed to connect: ") + e.what()
        );
    }
}

bool DBConnection::isConnected() const {
    return m_conn && m_conn->is_open();
}

void DBConnection::reconnect() {
    std::cerr << "[DBConnection] Reconnecting...\n";
    m_conn.reset();
    connect();
}

pqxx::connection& DBConnection::get() {
    if (!isConnected()) {
        throw std::runtime_error(
            "[DBConnection] get() called but connection is not open. "
            "Call connect() first."
        );
    }
    return *m_conn;
}