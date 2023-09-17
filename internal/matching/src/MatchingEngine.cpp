//
// Created by ArgonautDev on 9/17/2023.
//

#include "MatchingEngine.h"

namespace TradingEngine::Matching {

    void MatchingEngine::AddOrder(Data::Order &order, std::vector<Data::OrderAction> &actions) {
        CORE_TRACE("Received Order: {}", order);
        auto *ob = m_orderManager.GetOrderBook(order.SymbolId);
        // TODO: Return some kind of useful error
        if (ob == nullptr) {
            // TODO: Fix this. This is just here bcs we dont return an error and need to convey
            // somehow that the order wasnt actually persisted
            actions.clear();
            return;
        }

        // TODO: Rework that we don't use the orderbook directly
        // True if the match was complete -> Order either filled, or of kill variant
        bool finishedMatch = Match(order, *ob, actions);
        ob->ClearEmptyLevels();
        if (finishedMatch) {
            CORE_TRACE("FINISHED MATCH, REPORT CANCELLATION");
            return;
        }

        // Order has not been fully filled, thus place it.
        // TODO: This will search the OB again, improve
        m_orderManager.AddOrder(order, actions);
    }

    bool MatchingEngine::Match(Data::Order &order, Data::OrderBook &book, std::vector<Data::OrderAction> &actions) {
        switch (order.Type) {
            case Data::OrderType::MARKET:
                if (!MatchMarket(order, book, actions)) {
                    actions.emplace_back(Data::Action::CANCELLED, order, std::nullopt, 0);
//                        m_reporter.ReportOrderFill(order, order, Data::Action::CANCELLED);
                }
                return true;
            case Data::OrderType::LIMIT:
                return MatchIOC(order, book, actions);
            case Data::OrderType::FOK:
                MatchFOK(order, book, actions);
                return true;
            case Data::OrderType::IOC:
                if (!MatchIOC(order, book, actions)) {
//                        m_reporter.ReportOrderFill(order, order, Data::Action::CANCELLED);
                    actions.emplace_back(Data::Action::CANCELLED, order, std::nullopt, 0);
                }
                return true;
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

    void MatchingEngine::MatchFOK(Data::Order &order, Data::OrderBook &book, std::vector<Data::OrderAction> &actions) {
        if (order.Side == Data::OrderSide::BUY) {
            MatchFOK(order, book.Asks(), m_greater, actions);
        } else {
            MatchFOK(order, book.Bids(), m_less, actions);
        }
    }

    bool MatchingEngine::MatchMarket(Data::Order &order, Data::OrderBook &book, std::vector<Data::OrderAction> &actions) {
        // For now, we'll pretend a Market Order is basically an IOC with infinite price
        if (order.Side == Data::OrderSide::BUY) {
            order.Price = INT64_MAX;
            return MatchIOC(order, book.Asks(), m_greater, actions);
        } else {
            // let's not use a negative value here. We dont want the seller to have to pay
            // TODO: check correctness of this. Might be broken
            order.Price = 0;
            return MatchIOC(order, book.Bids(), m_less, actions);
        }
    }

    bool MatchingEngine::MatchIOC(Data::Order &order, Data::OrderBook &book, std::vector<Data::OrderAction> &actions) {
        if (order.Side == Data::OrderSide::BUY) {
            return MatchIOC(order, book.Asks(), m_greater, actions);
        } else {
            return MatchIOC(order, book.Bids(), m_less, actions);
        }
    }
}
