//
// Created by danie on 9/10/2022.
//

#ifndef TRADINGENGINE_MATCHINGENGINE_H
#define TRADINGENGINE_MATCHINGENGINE_H

#include <symbol.h>
#include <vector>
#include <unordered_map>
#include "order_book.h"

namespace TradingEngine::Server {

    class MatchingEngine {
    public:
        void AddOrder(Data::Order&& order);

    private:
        bool Match(Data::Order& order, Data::OrderBook& book);
        bool MatchMarket(Data::Order& order, Data::OrderBook& book);
        template<typename S, typename Comp>
        bool MatchIOC(Data::Order& order, S &levels, Comp& compare);

        std::greater<int64_t> m_greater{};
        std::less<int64_t> m_less{};

        std::vector<Data::Symbol> m_symbols{};
        std::unordered_map<uint32_t, Data::Symbol&> m_symbolsMap{};

        std::unordered_map<uint32_t, Data::OrderBook> m_orderBooks{};
        std::unordered_map<uint64_t, Data::Order> m_orders{};
    };

} // Server

#endif //TRADINGENGINE_MATCHINGENGINE_H
