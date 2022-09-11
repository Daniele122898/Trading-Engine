//
// Created by danie on 9/10/2022.
//

#include "MatchingEngine.h"

namespace TradingEngine::Server {
    // TODO: Add proper errors
    void MatchingEngine::AddOrder(Data::Order &&order) {
        auto obIt = m_orderBooks.find(order.SymbolId);
        if (obIt == m_orderBooks.end()) {
            return;
        }

        auto ob = obIt->second;
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
                break;
            case Data::OrderType::FOK:
                break;
            case Data::OrderType::IOC:
                break;
            case Data::OrderType::STOP_MARKET:
                break;
            case Data::OrderType::STOP_LIMIT:
                break;
            case Data::OrderType::QUOTE:
                break;
        }
        return false;
    }

    bool MatchingEngine::MatchMarket(Data::Order &order, Data::OrderBook &book) {
        if (order.Side == Data::OrderSide::BUY) {
            auto asks = book.Asks();
            for (auto lvl: asks) {

            }
        } else {

        }
        return true;
    }
} // Server