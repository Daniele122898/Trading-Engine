//
// Created by danie on 9/22/2022.
//

#ifndef TRADINGENGINE_ORDERREPORT_H
#define TRADINGENGINE_ORDERREPORT_H

#include <cstdint>

namespace TradingEngine::Matching {
    struct OrderReport {
        OrderReport(uint64_t orderId, uint64_t againstId, int64_t price, uint32_t quantity):
                OrderId{orderId}, AgainstId{againstId}, Price{price}, Quantity{quantity} {}

        uint64_t OrderId;
        uint64_t AgainstId;
        int64_t Price;
        uint32_t Quantity;
    };
}

#endif //TRADINGENGINE_ORDERREPORT_H
