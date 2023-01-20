#ifndef STATSENGINE_INSERTER_H
#define STATSENGINE_INSERTER_H

#include <cstdint>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <chrono>

#include "log.h"
#include "symbol.h"
#include "db/engineDb.h"
#include "db/statsDb.h"
#include "tm.h"

namespace StatsEngine {

    class Inserter {
    public:
        Inserter(std::vector<TradingEngine::Data::Symbol> symbols,
                std::string engineConnStr,
                std::string statsConnStr) : 
            m_symbols{std::move(symbols)}, m_engineDb(engineConnStr), m_statsDb(statsConnStr, engineConnStr) {

            tp = std::chrono::system_clock::now();

            m_thread = std::thread([this]() {
                HandlerLoop();
            });
        }

        ~Inserter() {
            m_running = false;
            // wait for thread to shut down
            CORE_TRACE("Waiting for inserter thread to exit");
            if (m_thread.joinable())
                m_thread.join();
            CORE_TRACE("Killed inserter thread");
        }

        Inserter(const Inserter &reporter) = delete;
        Inserter operator=(const Inserter &other) = delete;
        Inserter(Inserter &&reporter) = delete;
        Inserter &operator=(Inserter &&other) = delete;

    private:
        void HandlerLoop();
        std::chrono::system_clock::time_point tp;

        std::vector<TradingEngine::Data::Symbol> m_symbols;

        Db::EngineDb m_engineDb;
        Db::StatsDb m_statsDb;

        std::thread m_thread;
        bool m_running = true;
    };
}

#endif //STATSENGINE_INSERTER_H
