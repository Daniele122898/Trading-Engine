//
// Created by danie on 9/9/2022.
//

#include "order_book.h"

namespace TradingEngine::Data {
    void OrderBook::AddOrder(Order &order) {
        auto &lvl = findLevelOrAdd(order);
        lvl.AddOrder(order);

        m_orders[order.Id] = &order;
    }

    void OrderBook::RemoveOrder(uint64_t Id) {
        auto orderIt = m_orders.find(Id);
        if (orderIt == m_orders.end())
            return; //TODO: Potentially throw error

        auto &order = reinterpret_cast<Order &>(orderIt->second);
        // Assume if the order exists in m_orders, it is in a level
        auto &lvl = findLevelOrAdd(order);
        lvl.RemoveOrder(order);
        m_orders.erase(Id);
    }

    Level* OrderBook::BestBid() const {
        auto first = m_bids.begin();
        if (first == m_bids.end())
            return nullptr;

        return const_cast<Level *>(&(*first));
    }

    Level* OrderBook::BestAsk() const {
        auto first = m_asks.begin();
        if (first == m_asks.end())
            return nullptr;

        return const_cast<Level *>(&(*first));
    }

    // TODO: imrpove this entire function, it's bad (MVP)
    Level &OrderBook::findLevelOrAdd(Order &order) {
        // TODO: Improve this search!
        Level lvl(order.Price);
        if (order.Side == OrderSide::BUY) {
            auto lvlIt = m_bids.find(lvl);
            if (lvlIt == m_bids.end()) {
                auto ret = m_bids.insert(std::move(lvl));
                return const_cast<Level &>(*(ret.first));
            } else {
                return const_cast<Level &>(*lvlIt);
            }
        } else {
            auto lvlIt = m_asks.find(lvl);
            if (lvlIt == m_asks.end()) {
                auto ret = m_asks.insert(std::move(lvl));
                return const_cast<Level &>(*(ret.first));
            } else {
                return const_cast<Level &>(*lvlIt);
            }
        }
    }
} // Data