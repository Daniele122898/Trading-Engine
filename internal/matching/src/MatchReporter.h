//
// Created by danie on 9/19/2022.
//

#ifndef TRADINGENGINE_MATCHREPORTER_H
#define TRADINGENGINE_MATCHREPORTER_H

#include "OrderReport.h"

namespace TradingEngine::Matching {

    template<typename Logger, typename Persistence>
    class MatchReporter {
    public:
//        explicit MatchReporter(Impl implementation) : m_implementation{implementation} {}

        explicit MatchReporter(std::unique_ptr<Logger> logger, std::unique_ptr<Persistence> persistence) :
                m_logger{std::move(logger)}, m_persistence{std::move(persistence)} {}

        inline void ReportOrderFill(Data::Order const & order, Data::Order const & counterOrder, Data::FillReason reason, uint32_t diff = 0) {
            m_persistence->ReportOrderFill(order, counterOrder, reason, diff);
            m_logger->ReportOrderFill(order, counterOrder, reason, diff);
        }

    private:
        std::unique_ptr<Logger> m_logger;
        std::unique_ptr<Persistence> m_persistence;

    };

}

#endif //TRADINGENGINE_MATCHREPORTER_H
