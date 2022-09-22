//
// Created by danie on 9/22/2022.
//

#ifndef TRADINGENGINE_LOGORDERREPORTER_H
#define TRADINGENGINE_LOGORDERREPORTER_H

#include <cstdint>
#include "log.h"

namespace TradingEngine::Matching {

    class LogOrderReporter {
    public:
        void ReportOrderFill(uint64_t orderId, uint64_t againstId, int64_t price, uint32_t quantity) {
            CORE_INFO("FILL REPORT: {} against {}: {} x {}", orderId, againstId, price, quantity);
        }
    };

} // Server

#endif //TRADINGENGINE_LOGORDERREPORTER_H
