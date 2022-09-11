//
// Created by danie on 9/8/2022.
//

#ifndef TRADINGENGINE_ORDER_H
#define TRADINGENGINE_ORDER_H

#include <cstdint>

namespace TradingEngine::Data {
    enum class OrderType {
        MARKET,
        LIMIT,
        FOK,
        IOC,
        STOP_MARKET,
        STOP_LIMIT,
        QUOTE,
    };

    enum class OrderSide {
        BUY,
        SELL
    };

    enum class OrderLifetime {
        GFD,
        GTD,
        GTC
    };

    struct Order {

        uint64_t Id;
        uint32_t SymbolId;

        OrderType Type;
        OrderSide Side;
        OrderLifetime Lifetime;

        // prices can be negative
        int64_t Price;

        uint32_t InitialQuantity;
        uint32_t CurrentQuantity;

        [[nodiscard]]
        inline uint32_t QuantityLeft() const { return InitialQuantity - CurrentQuantity; }

        Order() = default;

        Order(uint64_t id,
              uint32_t symbolId,
              OrderType type,
              OrderSide side,
              OrderLifetime lifetime,
              int64_t price,
              uint32_t initialQuantity);

    };

} // TradingEngine::Data

#endif //TRADINGENGINE_ORDER_H
