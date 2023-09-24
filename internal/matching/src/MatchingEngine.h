//
// Created by danie on 9/10/2022.
//

#ifndef TRADINGENGINE_MATCHINGENGINE_H
#define TRADINGENGINE_MATCHINGENGINE_H

#include <cstdint>
#include <thread>
#include <vector>
#include <unordered_map>
#include <optional>
#include <list>

#include "order.h"
#include "readerwriterqueue.h"
#include "symbol.h"
#include "order_book.h"
#include "not_implemented_exception.h"
#include "log.h"
#include "OrderManager.h"

namespace TradingEngine::Matching {

    class MatchingEngine {
    public:

        explicit MatchingEngine(Data::OrderManager& orderManager) :
        m_orderManager{orderManager} {};

        MatchingEngine(const MatchingEngine &engine) = delete;
        // MatchingEngine(const MatchingEngine &&engine) = delete;
        MatchingEngine operator=(const MatchingEngine &engine) = delete;
        // MatchingEngine operator=(const MatchingEngine &&engine) = delete;

        void AddOrder(Data::Order &order, std::vector<Data::OrderAction>& actions);

    private:
        bool Match(Data::Order &order, Data::OrderBook &book, std::vector<Data::OrderAction>& actions);
        void MatchFOK(Data::Order &order, Data::OrderBook &book, std::vector<Data::OrderAction>& actions);

        template<template<class> class S, typename Comp>
        void MatchFOK(Data::Order &order, std::set<Data::Level, S<Data::Level>> const &levels, Comp &compare, std::vector<Data::OrderAction>& actions);

        bool MatchMarket(Data::Order &order, Data::OrderBook &book, std::vector<Data::OrderAction>& actions);
        bool MatchIOC(Data::Order &order, Data::OrderBook &book, std::vector<Data::OrderAction>& actions);

        template<typename S, typename Comp>
        bool MatchIOC(Data::Order &order, S &levels, Comp &compare, std::vector<Data::OrderAction>& actions);

        Data::OrderManager& m_orderManager;

        std::greater<int64_t> m_greater{};
        std::less<int64_t> m_less{};
    };

} // Server

#include "MatchingEngine.impl.h"

#endif //TRADINGENGINE_MATCHINGENGINE_H
