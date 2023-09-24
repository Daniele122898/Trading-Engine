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
        explicit OrderBook(const Symbol& symbol): mSymbol{symbol} {}

        [[nodiscard]]
        const Symbol& GetSymbol() const { return mSymbol; }
        [[nodiscard]]
        Level* BestBid() const;
        [[nodiscard]]
        Level* BestAsk() const;
        [[nodiscard]]
        auto& Bids() const { return mBids; }
        [[nodiscard]]
        auto& Asks() const { return mAsks; }

        void AddOrder(Order &order);
        void RemoveOrder(uint64_t Id);
        void ClearEmptyLevels();

    private:
        [[nodiscard]]
        Level& findLevelOrAdd(Order &order);

        struct Symbol mSymbol;

        std::set<Level, std::greater<Level>> mBids{};
        std::set<Level, std::less<Level>> mAsks{};
        std::unordered_map<uint64_t , Order> mOrders{};
    };

    std::ostream& operator<<(std::ostream& str, const OrderBook& book);

} // Data

#endif //TRADINGENGINE_ORDERBOOK_H
