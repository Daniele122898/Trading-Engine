//
// Created by danie on 9/10/2022.
//

#include "MatchingEngine.h"
#include <not_implemented_exception.h>
#include <log.h>

namespace TradingEngine::Server {
    // TODO: Add proper errors
    void MatchingEngine::AddOrder(Data::Order &&order) {
        CORE_TRACE("Received Order: {}", order);
        auto obIt = m_orderBooks.find(order.SymbolId);
        if (obIt == m_orderBooks.end()) {
            return;
        }

        auto ob = obIt->second;
        // First we add order, then we match

        // True if the match was complete -> Order either filled, or of kill variant
        if (Match(order, ob)) {
            return;
        }
    }

    // TODO: Add fill book to keep track of all the trades that actually happened lol
    bool MatchingEngine::Match(Data::Order &order, Data::OrderBook& book) {
        switch (order.Type) {
            case Data::OrderType::MARKET:
                return MatchMarket(order, book);
            case Data::OrderType::LIMIT:
                throw Util::not_implemented_exception();
            case Data::OrderType::FOK:
                throw Util::not_implemented_exception();
                break;
            case Data::OrderType::IOC:
                return MatchIOC(order, book);
                break;
            case Data::OrderType::STOP_MARKET:
                throw Util::not_implemented_exception();
                break;
            case Data::OrderType::STOP_LIMIT:
                throw Util::not_implemented_exception();
                break;
            case Data::OrderType::QUOTE:
                throw Util::not_implemented_exception();
                break;
        }
        return false;
    }

    bool MatchingEngine::MatchMarket(Data::Order &order, Data::OrderBook &book) {
        // For now, we'll pretend a Market Order is basically an IOC with infinite price
        if (order.Side == Data::OrderSide::BUY) {
            order.Price = INT64_MAX;
            return MatchIOC(order, book);
        } else {
            // let's not use a negative value here. We dont want the seller to have to pay
            // TODO: check correctness of this. Might be broken
            order.Price = 0;
            return MatchIOC(order, book);
        }
    }

    // Left off: maybe template this so we have the set given as well as the comparator for > or < of price
    bool MatchingEngine::MatchIOC(Data::Order &order, Data::OrderBook &book) {
        auto itBegin = order.Side == Data::OrderSide::BUY ? book.Asks().begin() : book.Bids().begin();
        auto itEnd = order.Side == Data::OrderSide::BUY ? book.Asks().end() : book.Bids().end();
        for (auto lvl = itBegin; lvl != itEnd; ++lvl) {
            auto& level = const_cast<Data::Level&>(*lvl);
            if (level.Price > )
            Data::OrderNode* curr = level.Head;
            while (curr != nullptr) {
                Data::Order& o = curr->Order;
                int64_t diff = std::min(order.CurrentQuantity, o.CurrentQuantity);
                order.CurrentQuantity -= diff;
                o.CurrentQuantity -= diff;

                Data::OrderNode* next = curr->Next;
                if (o.CurrentQuantity == 0) {
                    level.RemoveOrder(curr);
                } else {
                    level.DecreaseVolume(diff);
                }
                curr = next;

                if (order.CurrentQuantity > 0)
                    return true;
            }
        }

        if (order.CurrentQuantity > 0) {
            CORE_TRACE("Order with id {} could not be filled fully", order.Id);
        }
        return true;
    }
} // Server