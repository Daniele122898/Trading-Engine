//
// Created by danie on 11/28/2022.
//

#ifndef TRADINGENGINE_BROADCASTER_H
#define TRADINGENGINE_BROADCASTER_H

#include "crow.h"
#include <order.h>
#include <unordered_map>
#include <mutex>
#include <readerwriterqueue.h>
#include "dtos/WebsocketDto.h"
#include "log.h"
#include "Ratelimiter.h"

namespace TradingEngine {

    class Broadcaster {

    public:
        explicit Broadcaster(std::unordered_map<std::string, uint64_t> &apiKeys, Util::Ratelimiter &ratelimiter) :
                m_apiKeys{apiKeys}, m_ratelimiter{ratelimiter} {
            m_thread = std::thread([this]() {
                BroadcastingLoop();
            });
        }

        ~Broadcaster() {
            m_running = false;
            // wait for thread to shut down
            CORE_TRACE("Waiting for reporter thread to exit");
            if (m_thread.joinable())
                m_thread.join();
            CORE_TRACE("Killed reporter thread");
        }

        void ReportOrderCreation(Data::Order const &order);

        void ReportOrderFill(Data::Order const &order, Data::Order const &counterOrder, Data::Action reason,
                             uint32_t diff = 0);

        void OnOpen(crow::websocket::connection &conn);

        void OnError(crow::websocket::connection &conn);

        void OnClose(crow::websocket::connection &conn, const std::string &reason);

        void OnMessage(crow::websocket::connection &conn, const std::string &data, bool is_binary);

        void AddSymbol(uint32_t symbolId);

        Broadcaster(const Broadcaster &broadcaster) = delete;

        Broadcaster operator=(const Broadcaster &other) = delete;

        Broadcaster(Broadcaster &&broadcaster) = delete;

        Broadcaster &operator=(Broadcaster &&other) = delete;

    private:

        void BroadcastingLoop();

        std::unordered_map<std::string, uint64_t> &m_apiKeys;
        std::unordered_map<uint64_t, crow::websocket::connection *> m_users{};
        std::unordered_map<crow::websocket::connection *, std::vector<uint32_t>> m_connections{};
        std::unordered_map<uint32_t, std::vector<crow::websocket::connection *>> m_symbolsToUsers{};
        std::mutex m_mtx;

        moodycamel::BlockingReaderWriterQueue<WsData::ShareReport> m_reports{200};
        moodycamel::BlockingReaderWriterQueue<WsData::Creation> m_creations{200};
        std::thread m_thread;
        bool m_running = true;

        Util::Ratelimiter& m_ratelimiter;
    };

} // TradingEngine

#endif //TRADINGENGINE_BROADCASTER_H
