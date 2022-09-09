//
// Created by danie on 9/8/2022.
//

#include "order.h"

namespace TradingEngine::Data {

    Order::Order(uint64_t id, uint32_t symbolId, OrderType type, OrderSide side, OrderLifetime lifetime,
                 int64_t price, uint32_t initialQuantity) :
                 Id{id}, SymbolId{symbolId}, Type{type}, Side{side}, Lifetime{lifetime}, Price{price}, InitialQuantity{initialQuantity}, CurrentQuantity{initialQuantity} {
    }
} // Data