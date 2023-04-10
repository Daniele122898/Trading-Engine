//
// Created by danie on 11/28/2022.
//

#ifndef TRADINGENGINE_BROADCASTER_H
#define TRADINGENGINE_BROADCASTER_H

#include "crow.h"
#include <cstdint>
#include <order.h>
#include <unordered_map>
#include <mutex>
#include <readerwriterqueue.h>
#include <vector>
#include "crow/websocket.h"
#include "dtos/WebsocketDto.h"
#include "log.h"
#include "Ratelimiter.h"

namespace TradingEngine {

    class Broadcaster {

    public:
        explicit Broadcaster(std::unordered_map<std::string, uint64_t> &apiKeys, Util::Ratelimiter &ratelimiter) :
                m_apiKeys{apiKeys}, m_ratelimiter{ratelimiter} {
        }

        void ReportActions(std::vector<Data::OrderAction>& actions, uint32_t symbolId);

        void OnOpen(crow::websocket::connection &conn);
        void OnError(crow::websocket::connection &conn);
        void OnClose(crow::websocket::connection &conn, const std::string &reason);
        void OnMessage(crow::websocket::connection &conn, const std::string &data, bool is_binary);

        void AddSymbol(uint32_t symbolId);

    private:

        void OnLoginReguest(crow::websocket::connection& conn, const WsData::Payload& payload);
        void OnSubscribeReguest(crow::websocket::connection& conn, const WsData::Payload& payload);
        void OnUnSubscribeReguest(crow::websocket::connection& conn, const WsData::Payload& payload);

        std::unordered_map<std::string, uint64_t> &m_apiKeys;
        std::unordered_map<uint64_t, crow::websocket::connection *> m_users{};
        std::unordered_map<crow::websocket::connection *, std::vector<uint32_t>> m_connections{};
        std::unordered_map<uint32_t, std::vector<crow::websocket::connection *>> m_symbolsToUsers{};
        std::mutex m_mtx;

        Util::Ratelimiter& m_ratelimiter;
    };

} // TradingEngine

#endif //TRADINGENGINE_BROADCASTER_H
