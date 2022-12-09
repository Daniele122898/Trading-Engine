//
// Created by danie on 12/7/2022.
//

#include "EODHandler.h"

namespace TradingEngine {

    void EODHandler::HandlerLoop() {
        while (m_running) {

            auto now = std::chrono::system_clock::now();

            if (now <= tp) {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                continue;
            }

            tp += std::chrono::hours(24);

            auto expiredIds = m_db.GetExpiredOrders();
            if (expiredIds.empty())
                continue;

            for (uint64_t orderId : expiredIds) {
                auto order = m_removeOrderFunc(orderId);
                m_db.AddFill(order, order, Data::FillReason::EXPIRED);
                CORE_TRACE("EXPIRED ORDER {}", order.Id);
            }

        }
    }
} // TradingEngine