//
// Created by danie on 9/8/2022.
//

#include "order.h"

namespace TradingEngine::Data {

    Order::Order(uint64_t id, uint64_t userId, uint32_t symbolId, OrderType type, OrderSide side, OrderLifetime lifetime,
                 int64_t price, uint32_t initialQuantity) :
                 Id{id}, UserId{userId}, SymbolId{symbolId}, Type{type}, Side{side}, Lifetime{lifetime}, Price{price}, InitialQuantity{initialQuantity}, CurrentQuantity{initialQuantity} {
    }

    Order::Order(uint64_t id, uint64_t userId, uint32_t symbolId, OrderType type, OrderSide side, OrderLifetime lifetime,
                 int64_t price, uint32_t initialQuantity, uint32_t currentQuantity,
                 std::chrono::milliseconds expiryMs,
                 std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> creationTp) :
            Id{id}, UserId{userId}, SymbolId{symbolId}, Type{type},
            Side{side}, Lifetime{lifetime}, Price{price}, InitialQuantity{initialQuantity},
            CurrentQuantity{currentQuantity}, ExpiryMs{expiryMs}, CreationTp{creationTp} {
    }

    std::ostream& operator<<(std::ostream& str, const OrderLifetime& lifetime) {
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

    std::ostream& operator<<(std::ostream& str, const OrderType& type) {
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

    std::ostream& operator<<(std::ostream& str, const Order& order) {
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
} // Data