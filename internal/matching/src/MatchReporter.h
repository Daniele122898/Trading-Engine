//
// Created by danie on 9/19/2022.
//

#ifndef TRADINGENGINE_MATCHREPORTER_H
#define TRADINGENGINE_MATCHREPORTER_H

#include "OrderReport.h"
#include "order.h"
#include <cstdint>
#include <memory>

namespace TradingEngine::Matching {

    template<typename Logger, typename Persistence, typename Broadcaster>
    class MatchReporter {
    public:
//        explicit MatchReporter(Impl implementation) : m_implementation{implementation} {}

        explicit MatchReporter(std::unique_ptr<Logger> logger, std::unique_ptr<Persistence> persistence,
                               std::shared_ptr<Broadcaster> broadcaster) :
                m_logger{std::move(logger)}, m_persistence{std::move(persistence)}, m_broadcaster{broadcaster} {}

        inline void UpdateOrder(Data::Order const & order, uint32_t newQuant) {
            m_persistence->UpdateOrderQuantity(order, newQuant);
        }

        inline void ReportOrderFill(Data::Order const & order, Data::Order const & counterOrder, Data::Action reason, uint32_t diff = 0) {
            m_persistence->ReportOrderFill(order, counterOrder, reason, diff);
            m_broadcaster->ReportOrderFill(order, counterOrder, reason, diff);
            m_logger->ReportOrderFill(order, counterOrder, reason, diff);
        }

    private:
        std::unique_ptr<Logger> m_logger;
        std::unique_ptr<Persistence> m_persistence;
        std::shared_ptr<Broadcaster> m_broadcaster;

    };

}

#endif //TRADINGENGINE_MATCHREPORTER_H
