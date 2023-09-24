//
// Created by danie on 12/7/2022.
//

#include "EODHandler.h"

namespace TradingEngine {

    void EODHandler::HandlerLoop() {
        while (mRunning) {

            auto now = std::chrono::system_clock::now();

            if (now <= tp) {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                continue;
            }

            tp += std::chrono::hours(24);

            auto orders = mOrderManager.RemoveExpiredOrders(now);
        }
    }
} // TradingEngine
