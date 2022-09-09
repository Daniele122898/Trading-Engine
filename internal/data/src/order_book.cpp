//
// Created by danie on 9/9/2022.
//

#include "order_book.h"

namespace TradingEngine::Data {
    void OrderBook::AddOrder(Order* order) {
        Level* level;
        if (auto lvl = m_levels.find(order->Price); lvl != m_levels.end()) {
            level = lvl->second;
            level->AddOrder(order);
        } else {
            level = new Level(order->Price, order);
            // Add to data
            m_levels[order->Price] = level;
            if (order->Side == OrderSide::BUY)
                m_bids.insert(level);
            else
                m_asks.insert(level);
        }
        m_orders[order->Id] = order;
    }

    void OrderBook::RemoveOrder(uint64_t Id) {
        auto orderIt = m_orders.find(Id);
        if (orderIt == m_orders.end())
            return; //TODO: Potentially throw error

        auto order = orderIt->second;
        // Assume if the order exists in m_orders, it is in a level
        auto lvl = m_levels[order->Price];
        lvl->RemoveOrder(order);
        m_orders.erase(Id);
    }

    Level *OrderBook::BestBid() const {
        auto first = m_bids.begin();
        return first == m_bids.end() ? nullptr : *first;
    }

    Level *OrderBook::BestAsk() const {
        auto first = m_asks.begin();
        return first == m_asks.end() ? nullptr : *first;
    }
} // Data