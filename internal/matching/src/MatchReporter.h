//
// Created by danie on 9/19/2022.
//

#ifndef TRADINGENGINE_MATCHREPORTER_H
#define TRADINGENGINE_MATCHREPORTER_H

#include "OrderReport.h"

namespace TradingEngine::Matching {

    template<typename Impl>
    class MatchReporter {
    public:
//        explicit MatchReporter(Impl implementation) : m_implementation{implementation} {}

        explicit MatchReporter(std::unique_ptr<Impl> implementation) :
                m_implementation{std::move(implementation)} {}

        inline void ReportOrderFill(OrderReport report) {
            m_implementation->ReportOrderFill(report);
        }

    private:
        std::unique_ptr<Impl> m_implementation;

    };

}

#endif //TRADINGENGINE_MATCHREPORTER_H
