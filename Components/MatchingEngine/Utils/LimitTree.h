#pragma once

#include "limNode.h"
#include <cmath>

// ─── LimitTree ────────────────────────────────────────────────────────────
//  A binary search tree of Limit nodes, ordered by price.
//  One LimitTree instance per side of the book (buy and sell each get their
//  own — same class, no inheritance needed since the only difference is
//  which end callers read as "best").
//
//  This is a plain unbalanced BST for clarity. In production you'd want
//  a self-balancing tree (red-black / AVL) to guarantee O(log n) worst case
//  instead of O(n) on adversarial/sorted insert order — note that below.
//
class LimitTree {
private:
    Limit* root;
    int    nodeCount;

    // Recursive insert helper — returns the (possibly new) subtree root.
    Limit* insertRec(Limit* node, Limit* newLimit) {
        if (!node) return newLimit;

        if (newLimit->limitPrice < node->limitPrice) {
            node->left = insertRec(node->left, newLimit);
        } else if (newLimit->limitPrice > node->limitPrice) {
            node->right = insertRec(node->right, newLimit);
        }
        // equal price should never reach here — caller checks limitMap first
        return node;
    }

    Limit* findRec(Limit* node, double price) const {
        if (!node) return nullptr;
        if (price == node->limitPrice) return node;
        if (price < node->limitPrice) return findRec(node->left, price);
        return findRec(node->right, price);
    }

    // Standard BST delete — returns the (possibly new) subtree root.
    Limit* removeRec(Limit* node, double price) {
        if (!node) return nullptr;

        if (price < node->limitPrice) {
            node->left = removeRec(node->left, price);
        } else if (price > node->limitPrice) {
            node->right = removeRec(node->right, price);
        } else {
            // found it
            if (!node->left) {
                Limit* rightChild = node->right;
                delete node;
                --nodeCount;
                return rightChild;
            }
            if (!node->right) {
                Limit* leftChild = node->left;
                delete node;
                --nodeCount;
                return leftChild;
            }
            // two children: replace with in-order successor (min of right subtree)
            Limit* successor = node->right;
            while (successor->left) successor = successor->left;

            node->limitPrice  = successor->limitPrice;
            node->headOrder   = successor->headOrder;
            node->tailOrder   = successor->tailOrder;
            node->orderCount  = successor->orderCount;
            node->totalVolume = successor->totalVolume;

            node->right = removeRec(node->right, successor->limitPrice);
        }
        return node;
    }

public:
    LimitTree() : root(nullptr), nodeCount(0) {}

    // Non-copyable — owns raw pointers, copying would double-free.
    LimitTree(const LimitTree&)            = delete;
    LimitTree& operator=(const LimitTree&) = delete;

    ~LimitTree() {
        clear(root);
    }

    void clear(Limit* node) {
        if (!node) return;
        clear(node->left);
        clear(node->right);
        delete node;
    }

    int size() const { return nodeCount; }
    bool empty() const { return root == nullptr; }

    // Insert a new Limit node. Caller (OrderBook) must have already
    // checked limitMap to confirm this price doesn't already exist —
    // this method assumes the price is new.
    void insert(Limit* newLimit) {
        root = insertRec(root, newLimit);
        ++nodeCount;
    }

    // O(log n) average, O(n) worst case (unbalanced tree, sorted insert order)
    Limit* find(double price) const {
        return findRec(root, price);
    }

    void remove(double price) {
        root = removeRec(root, price);
    }

    // Lowest price in the tree — this is the BEST ASK when used as the
    // sell-side tree (cheapest seller wins).
    Limit* getMin() const {
        if (!root) return nullptr;
        Limit* node = root;
        while (node->left) node = node->left;
        return node;
    }

    // Highest price in the tree — this is the BEST BID when used as the
    // buy-side tree (highest buyer wins).
    Limit* getMax() const {
        if (!root) return nullptr;
        Limit* node = root;
        while (node->right) node = node->right;
        return node;
    }
};