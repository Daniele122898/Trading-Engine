//
// Created by danie on 9/9/2022.
//

#ifndef TRADINGENGINE_ORDERBOOK_H
#define TRADINGENGINE_ORDERBOOK_H

#include <set>
#include <unordered_map>
#include <optional>
#include "symbol.h"
#include "level.h"

namespace TradingEngine::Data {

    class OrderBook {
    public:
        explicit OrderBook(Symbol symbol): m_symbol{symbol} {}

        [[nodiscard]]
        const Symbol& Symbol() const { return m_symbol; }
        [[nodiscard]]
        Level* BestBid() const;
        [[nodiscard]]
        Level* BestAsk() const;
        [[nodiscard]]
        auto& Bids() const { return m_bids; }
        [[nodiscard]]
        auto& Asks() const { return m_asks; }

        void AddOrder(Order &order);
        void RemoveOrder(uint64_t Id);

    private:
        [[nodiscard]]
        Level& findLevelOrAdd(Order &order);

        struct Symbol m_symbol;

        std::set<Level, std::greater<>> m_bids{};
        std::set<Level, std::less<>> m_asks{};
        std::unordered_map<uint64_t , Order*> m_orders{};
    };

} // Data

#endif //TRADINGENGINE_ORDERBOOK_H
