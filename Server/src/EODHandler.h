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

namespace TradingEngine {

    class EODHandler {
    public:
        explicit EODHandler(std::string const & dbConnectionString, std::function<Data::Order(uint64_t)> removeOrderFunc) :
                m_db(dbConnectionString), m_removeOrderFunc{std::move(removeOrderFunc)} {

            tp = Util::GetPointInToday(23, 05, 0);

            m_thread = std::thread([this]() {
                HandlerLoop();
            });
        }

        ~EODHandler() {
            m_running = false;
            // wait for thread to shut down
            CORE_TRACE("Waiting for reporter thread to exit");
            if (m_thread.joinable())
                m_thread.join();
            CORE_TRACE("Killed reporter thread");
        }

        EODHandler(const EODHandler &reporter) = delete;

        EODHandler operator=(const EODHandler &other) = delete;

        EODHandler(EODHandler &&reporter) = delete;

        EODHandler &operator=(EODHandler &&other) = delete;

    private:
        void HandlerLoop();

        std::chrono::system_clock::time_point tp;
        std::thread m_thread;
        bool m_running = true;

        Db::Database m_db;
        std::function<Data::Order(uint64_t)> m_removeOrderFunc;
    };

} // TradingEngine

#endif //TRADINGENGINE_EODHANDLER_H
