//
// Created by danie on 9/8/2022.
//

#ifndef TRADINGENGINE_ORDER_H
#define TRADINGENGINE_ORDER_H

#include <cstdint>
#include <iostream>
#include <chrono>
#include <optional>

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

    enum class Action {
        CREATION,
        SELF_TRADE,
        CANCELLED,
        FILLED,
        EXPIRED
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
        uint64_t UserId;
        uint32_t SymbolId;

        OrderType Type;
        OrderSide Side;
        OrderLifetime Lifetime;

        // prices can be negative
        int64_t Price;

        uint32_t InitialQuantity;
        uint32_t CurrentQuantity;

        // TODO: Implement expiry
        // Expiry in ms since UTC Epoch 1970-01-01 00:00.000 UTC
//        std::chrono::duration<int64_t, std::ratio<1, 1000000000>> ExpiryMs;
        std::chrono::milliseconds ExpiryMs;
        std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> CreationTp;

        [[nodiscard]]
        inline uint32_t QuantityLeft() const { return InitialQuantity - CurrentQuantity; }

        Order() = default;

        Order(uint64_t id,
              uint64_t userId,
              uint32_t symbolId,
              OrderType type,
              OrderSide side,
              OrderLifetime lifetime,
              int64_t price,
              uint32_t initialQuantity);

        Order(uint64_t id,
              uint64_t userId,
              uint32_t symbolId,
              OrderType type,
              OrderSide side,
              OrderLifetime lifetime,
              int64_t price,
              uint32_t initialQuantity,
              uint32_t currentQuantity,
              std::chrono::milliseconds expiryMs,
              std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> creationTp);

    };

    std::ostream& operator<<(std::ostream& str, const OrderLifetime& lifetime);
    std::ostream& operator<<(std::ostream& str, const OrderType& type);
    std::ostream& operator<<(std::ostream& str, const Order& order);

    struct OrderAction {
        Action reason;
        Order order;
        std::optional<Order> counterOrder;
        uint32_t quantity;

        OrderAction(Action reason, Order order, std::optional<Order> counterOrder, uint32_t quantity) :
            reason{reason}, order{order}, counterOrder{counterOrder}, quantity{quantity} {}
    };
} // TradingEngine::Data



#endif //TRADINGENGINE_ORDER_H
