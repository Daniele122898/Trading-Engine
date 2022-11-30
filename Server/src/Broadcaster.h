//
// Created by danie on 11/28/2022.
//

#ifndef TRADINGENGINE_BROADCASTER_H
#define TRADINGENGINE_BROADCASTER_H

#include "crow.h"
#include <order.h>
#include <unordered_map>
#include <mutex>

namespace TradingEngine {

    class Broadcaster {
    public:
        explicit Broadcaster(std::unordered_map<std::string, uint64_t>& apiKeys): m_apiKeys{apiKeys} {}

        void ReportOrderFill(Data::Order const & order, Data::Order const & counterOrder, Data::FillReason reason, uint32_t diff = 0);

        void OnOpen(crow::websocket::connection& conn);
        void OnError(crow::websocket::connection& conn);
        void OnClose(crow::websocket::connection& conn, const std::string& reason);
        void OnMessage(crow::websocket::connection& conn, const std::string& data, bool is_binary);

        void AddSymbol(uint32_t symbolId);
    private:

        std::unordered_map<std::string, uint64_t>& m_apiKeys;
        std::unordered_map<uint64_t, crow::websocket::connection*> m_users{};
        std::unordered_map<crow::websocket::connection*, std::vector<uint32_t>> m_connections{};
        std::unordered_map<uint32_t, std::vector<crow::websocket::connection*>> m_symbolsToUsers{};
        std::mutex m_mtx;
    };

} // TradingEngine

#endif //TRADINGENGINE_BROADCASTER_H
