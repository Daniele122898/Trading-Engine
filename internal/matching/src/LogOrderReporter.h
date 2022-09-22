//
// Created by danie on 9/22/2022.
//

#ifndef TRADINGENGINE_LOGORDERREPORTER_H
#define TRADINGENGINE_LOGORDERREPORTER_H

#include <OrderReport.h>
#include <log.h>

namespace TradingEngine::Matching {

    class LogOrderReporter {
    public:
        void ReportOrderFill(OrderReport report) {
            CORE_INFO("FILL REPORT: {} against {}: {} x {}",
                      report.OrderId,
                      report.AgainstId,
                      report.Price,
                      report.Quantity);
        }
    };

} // Matching

#endif //TRADINGENGINE_LOGORDERREPORTER_H
