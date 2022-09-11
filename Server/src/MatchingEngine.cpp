//
// Created by danie on 9/10/2022.
//

#include "MatchingEngine.h"
#include <not_implemented_exception.h>

namespace TradingEngine::Server {
    // TODO: Add proper errors
    void MatchingEngine::AddOrder(Data::Order &&order) {
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
            order.Price = 0;
            return MatchIOC(order, book);
        }
    }

    bool MatchingEngine::MatchIOC(Data::Order &order, Data::OrderBook &book) {
        return true;
    }
} // Server