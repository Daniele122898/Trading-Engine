//
// Created by danie on 9/19/2022.
//

#ifndef TRADINGENGINE_MATCHREPORTER_H
#define TRADINGENGINE_MATCHREPORTER_H

#include <cstdint>

namespace TradingEngine::Matching {

    template<typename Impl>
    class MatchReporter {
    public:
        explicit MatchReporter(Impl implementation) : m_implementation{implementation} {}

        inline void ReportOrderFill(uint64_t orderId, uint64_t againstId, int64_t price, uint32_t quantity) {
            m_implementation.ReportOrderFill(orderId, againstId, price, quantity);
        }

    private:
        Impl m_implementation;

    };

}

#endif //TRADINGENGINE_MATCHREPORTER_H
