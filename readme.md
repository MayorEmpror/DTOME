# 📈 Distributed Trading & Order Matching Engine (DTOME)

DTOME is a high-performance, multithreaded C++ trading system that simulates a real-world stock/crypto exchange. It includes a matching engine, networked client-server architecture, and a trading terminal UI concept inspired by professional trading platforms.

---

## 🚀 Project Overview

DTOME is designed as a systems-level engineering project that demonstrates:

- Real-time order matching engine
- Multi-threaded concurrent processing
- TCP-based client-server communication
- Custom trading protocol design
- Scalable order book architecture
- Future-ready Qt trading terminal UI (planned)

---

## 🧠 Core Features

### ⚙️ Matching Engine
- Price-time priority order matching
- Limit & Market order support
- Efficient order book structure per asset
- Fast trade execution logic

### 🌐 Networking Layer
- TCP socket-based server
- Multiple client connections
- Custom protocol for order submission & trade execution

### 🧵 Concurrency System
- Multi-threaded architecture
- Thread-safe order book updates
- Producer-consumer order queue model
- Condition variables for event-driven execution

### 📊 Trading System Components
- Order management system
- Trade history tracking
- Portfolio simulation (planned)
- Market data streaming (planned)

---

## 📡 Custom Protocol

Example client-server messages:
