//
// Created by danie on 12/1/2022.
//

#ifndef TRADINGENGINE_MOCKREPORTER_H
#define TRADINGENGINE_MOCKREPORTER_H

#include <cstdint>
#include "order.h"

class MockReporter {
public:
    void ReportOrderFill(TradingEngine::Data::Order const & order,
                         TradingEngine::Data::Order const & counterOrder,
                         TradingEngine::Data::FillReason reason, uint32_t diff = 0) {
    }
    
    void UpdateOrderQuantity(TradingEngine::Data::Order const & order, uint32_t newQuant) {

    }
};

#endif //TRADINGENGINE_MOCKREPORTER_H
