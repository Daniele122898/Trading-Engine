//
// Created by danie on 9/8/2022.
//

#ifndef TRADINGENGINE_ORDER_H
#define TRADINGENGINE_ORDER_H

#include <cstdint>
#include <iostream>

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

    inline std::ostream& operator<<(std::ostream& str, const OrderLifetime& lifetime) {
        switch (lifetime) {
            case OrderLifetime::GFD:
                str << "GFD";
                break;
            case OrderLifetime::GTD:
                str << "GTD";
                break;
            case OrderLifetime::GTC:
                str << "GTC";
                break;
        }
        return str;
    }

    inline std::ostream& operator<<(std::ostream& str, const OrderType& type) {
        switch (type) {
            case OrderType::MARKET:
                str << "MARKET";
                break;
            case OrderType::LIMIT:
                str << "LIMIT";
                break;
            case OrderType::FOK:
                str << "FOK";
                break;
            case OrderType::IOC:
                str << "IOC";
                break;
            case OrderType::STOP_MARKET:
                str << "STOP_MARKET";
                break;
            case OrderType::STOP_LIMIT:
                str << "STOP_LIMIT";
                break;
            case OrderType::QUOTE:
                str << "QUOTE";
                break;
        }
        return str;
    }

    inline std::ostream& operator<<(std::ostream& str, const Order& order) {
        str << "Order ID: " << order.Id
        << "\nSymbol ID: " << order.SymbolId
        << "\nSide: " << (order.Side == OrderSide::BUY ? "BUY" : "SELL")
        << "\nType: " << order.Type
        << "\nLife Time: " << order.Lifetime
        << "\nPrice: " << order.Price
        << "\nInitial Q: " << order.InitialQuantity
        << "\nCurrent Q: " << order.CurrentQuantity;

        return str;
    }

} // TradingEngine::Data

#endif //TRADINGENGINE_ORDER_H
