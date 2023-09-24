//
// Created by danie on 12/7/2022.
//

#ifndef TRADINGENGINE_EODHANDLER_H
#define TRADINGENGINE_EODHANDLER_H

#include <log.h>
#include <thread>
#include <chrono>
#include <utility>
#include <ctime>
#include "db/db.h"
#include "tm.h"
#include "OrderManager.h"

namespace TradingEngine {

    class EODHandler {
    public:
        explicit EODHandler(std::string const & dbConnectionString, Data::OrderManager& orderManager) :
                mDb(dbConnectionString), mOrderManager{orderManager} {

            tp = Util::GetPointInToday(23, 05, 0);

            mThread = std::thread([this]() {
                HandlerLoop();
            });
        }

        ~EODHandler() {
            mRunning = false;
            // wait for thread to shut down
            CORE_TRACE("Waiting for EOD thread to exit");
            if (mThread.joinable())
                mThread.join();
            CORE_TRACE("Killed EOD thread");
        }

        EODHandler(const EODHandler &reporter) = delete;

        EODHandler operator=(const EODHandler &other) = delete;

        EODHandler(EODHandler &&reporter) = delete;

        EODHandler &operator=(EODHandler &&other) = delete;

    private:
        void HandlerLoop();

        std::chrono::system_clock::time_point tp;
        std::thread mThread;
        bool mRunning = true;

        Db::Database mDb;
        Data::OrderManager& mOrderManager;
    };

} // TradingEngine

#endif //TRADINGENGINE_EODHANDLER_H
