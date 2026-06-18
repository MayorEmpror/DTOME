#pragma once

class Order;  // Forward declaration

class Limit {
public:
    double limitPrice;

    // FIFO queue of orders at this price level
    Order* headOrder;
    Order* tailOrder;

    // Aggregate statistics
    int orderCount;
    int totalVolume;

    // BST links
    Limit* left;
    Limit* right;

    explicit Limit(double price)
        : limitPrice(price),
          headOrder(nullptr),
          tailOrder(nullptr),
          orderCount(0),
          totalVolume(0),
          left(nullptr),
          right(nullptr)
    {}
};