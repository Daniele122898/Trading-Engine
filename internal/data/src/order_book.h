//
// Created by danie on 9/9/2022.
//

#ifndef TRADINGENGINE_ORDERBOOK_H
#define TRADINGENGINE_ORDERBOOK_H

#include <set>
#include <unordered_map>
#include "symbol.h"
#include "level.h"

namespace TradingEngine::Data {

    class OrderBook {
    public:
        [[nodiscard]]
        const Symbol& Symbol() const { return m_symbol; }
        [[nodiscard]]
        Level* BestBid() const { return *m_bids.begin(); }
        [[nodiscard]]
        Level* BestAsk() const { return *m_asks.begin(); }
        [[nodiscard]]
        auto Bids() const { return m_bids; }
        [[nodiscard]]
        auto Asks() const { return m_asks; }

        void AddOrder(Order* order);
        void RemoveOrder(uint64_t Id);

    private:
        struct Symbol m_symbol;

        std::set<Level*, std::greater<>> m_bids;
        std::set<Level*, std::less<>> m_asks;
        // Memory is cheap, keep both data structures as they're good for their individual tasks
        std::unordered_map<int64_t, Level*> m_levels;
        std::unordered_map<uint64_t , Order*> m_orders;
    };

} // Data

#endif //TRADINGENGINE_ORDERBOOK_H
