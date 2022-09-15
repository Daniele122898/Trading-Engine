//
// Created by danie on 9/9/2022.
//

#include "order_book.h"
#include <string>

namespace TradingEngine::Data {
    void OrderBook::AddOrder(Order &order) {
        auto &lvl = findLevelOrAdd(order);

        m_orders[order.Id] = order;

        lvl.AddOrder(m_orders[order.Id]);
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

    // TODO: improve this entire function, it's bad (MVP)
    Level &OrderBook::findLevelOrAdd(Order &order) {
        // TODO: Improve this search!
        Level lvl(order.Price);
        if (order.Side == OrderSide::BUY) {
            auto lvlIt = m_bids.find(lvl);
            if (lvlIt == m_bids.end()) {
                auto ret = m_bids.emplace(order.Price);
                return const_cast<Level &>(*(ret.first));
            } else {
                return const_cast<Level &>(*lvlIt);
            }
        } else {
            auto lvlIt = m_asks.find(lvl);
            if (lvlIt == m_asks.end()) {
                auto ret = m_asks.emplace(order.Price);
                return const_cast<Level &>(*(ret.first));
            } else {
                return const_cast<Level &>(*lvlIt);
            }
        }
    }

    void OrderBook::ClearEmptyLevels() {
        // Check Asks
        int toRemove = 0;
        for (auto& lvl:m_asks) {
            if (lvl.IsEmpty())
                ++toRemove;
            else
                break;
        }
        if (toRemove > 0)
            m_asks.erase(m_asks.begin(), std::next(m_asks.begin(), toRemove));

        // Check bids
        toRemove = 0;
        for (auto& lvl:m_bids) {
            if (lvl.IsEmpty())
                ++toRemove;
            else
                break;
        }
        if (toRemove > 0)
            m_bids.erase(m_bids.begin(), std::next(m_bids.begin(), toRemove));
    }

    std::ostream& operator<<(std::ostream& str, const OrderBook& book) {
        int width = 30;
        std::string sep = std::string(width, '-');

        str << "\t\t" << book.Symbol().Ticker << '\n';
        str << sep << '\n';

        auto writeLine = [&] (std::string&& left, std::string&& mid, std::string&& right) {
            int lineSpace = (width / 3) -2;
            // left
            std::string spacer = std::string(lineSpace - left.size(), ' ');
            str << "| " << left << spacer;
            // mid
            spacer = std::string(lineSpace-mid.size(), ' ');
            str << "| " << mid << spacer;
            // right
            spacer = std::string(lineSpace-right.size(), ' ');
            str << "| " << right << spacer;
            str << "|\n";
        };

        writeLine("bid Vol", "price", "ask vol");
        str << sep << '\n';
        auto& asks = book.Asks();
        for (auto ask= asks.rbegin(); ask != asks.rend(); ++ask) {
            auto& askLvl = *ask;
            writeLine(std::to_string(0), std::to_string(askLvl.Price), std::to_string(askLvl.TotalVolume));
            str << sep << '\n';
        }

        auto& bids = book.Bids();
        for (auto bid= bids.begin(); bid != bids.end(); ++bid) {
            auto& bidLvl = *bid;
            writeLine(std::to_string(bidLvl.TotalVolume), std::to_string(bidLvl.Price), std::to_string(0));
            str << sep << '\n';
        }

        return str;
    }
} // Data