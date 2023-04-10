//
// Created by danie on 11/28/2022.
//

#include "Broadcaster.h"
#include "dtos/OrderActionDto.h"
#include "dtos/WebsocketDto.h"
#include "not_implemented_exception.h"
#include <exception>
#include <log.h>
#include <nlohmann/json.hpp>
#include <utility>

namespace TradingEngine {

    inline void Ack(crow::websocket::connection &conn, WsData::OpCodes opcode) {
        WsData::Ack ack{opcode};
        nlohmann::json j = ack;
        conn.send_text(j.dump());
    }

    inline void Respond(crow::websocket::connection &conn, WsData::OpCodes opcode, nlohmann::json data) {
        WsData::Payload payload{opcode, std::move(data)};
        nlohmann::json j = payload;
        conn.send_text(j.dump());
    }

    inline void RespondError(crow::websocket::connection &conn, WsData::ErrorCodes code, std::string erorrMsg) {
        Respond(conn, WsData::OpCodes::ERROR, WsData::Error{code, std::move(erorrMsg)});
    }

    inline bool
    IsLoggedIn(crow::websocket::connection &conn, std::unordered_map<uint64_t, crow::websocket::connection *> &users) {
        auto it = users.find((uint64_t) conn.userdata());
        if (it == users.end()) {
            RespondError(conn, WsData::ErrorCodes::UNAUTHORIZED,
                         "You have to log into the websocket before anything else!");
            return false;
        }
        return true;
    }

    inline bool IsRatelimited(crow::websocket::connection &conn, Util::Ratelimiter& ratelimiter) {
        auto userId = (uint64_t) conn.userdata();
        if (ratelimiter.IsRatelimited(Util::BUCKET_TYPE::SIMPLE, userId)) {
            RespondError(conn, WsData::ErrorCodes::RATELIMITED,
                         "You have sent too many requests!");
            return true;
        }
        return false;
    }

    void Broadcaster::OnOpen(crow::websocket::connection &conn) {
        CORE_TRACE("Opened Websocket connection from {}", conn.get_remote_ip());
        std::lock_guard<std::mutex> _(m_mtx);
        m_connections.emplace(&conn, std::vector<uint32_t>{});
    }

    void Broadcaster::OnError(crow::websocket::connection &conn) {
        CORE_TRACE("Error in Websocket connection from {}", conn.get_remote_ip());
    }

    void Broadcaster::OnClose(crow::websocket::connection &conn, const std::string &reason) {
        CORE_TRACE("Closed Websocket connection from {} because of {}", conn.get_remote_ip(), reason);
        std::lock_guard<std::mutex> _(m_mtx);
        uint64_t userId = (uint64_t) conn.userdata();
        m_users.erase(userId);
        auto u = m_connections.find(&conn);
        for (auto sid: u->second) {
            // remove ws from symbol list
            auto sit = m_symbolsToUsers.find(sid);
            auto wsconn = std::find(sit->second.begin(), sit->second.end(), &conn);

            auto wsend = --sit->second.end();
            crow::websocket::connection *wstemp = *wsend;
            *wsend = *wsconn;
            *wsconn = wstemp;
            sit->second.pop_back();
        }
        m_connections.erase(&conn);
    }

    void Broadcaster::OnMessage(crow::websocket::connection &conn, const std::string &data, bool is_binary) {
        if (is_binary) {
            RespondError(conn, WsData::ErrorCodes::INVALID_FORMAT, "Provide JSON formatted data");
            return;
        }

        try {
            auto json = nlohmann::json::parse(data);
            WsData::Payload request = json.get<WsData::Payload>();

            switch (request.opcode) {
                case WsData::OpCodes::LOGIN: {
                    OnLoginReguest(conn, request);
                    return;
                }
                case WsData::OpCodes::SUBSCRIBE: {
                    OnSubscribeReguest(conn, request);
                    return;
                }
                case WsData::OpCodes::UNSUBSCRIBE: {
                    OnUnSubscribeReguest(conn, request);
                    return;
                }
                default: {
                    RespondError(conn, WsData::ErrorCodes::INVALID_JSON, "Provide proper JSON formatted data");
                    return;
                }
            }

        } catch (nlohmann::json::parse_error &ex) {
            CORE_ERROR("FAILED TO PARSE JSON {}\n at byte {}\n{}", data, ex.byte, ex.what());
            Respond(conn, WsData::OpCodes::ERROR,
                    WsData::Error{WsData::ErrorCodes::INVALID_JSON, "Provide proper JSON formatted data"});
        } catch (std::exception const &e) {
            CORE_ERROR("ERROR: Data recieved \"{}\" \n exception: {}", data, e.what());
        }
    }

    void Broadcaster::AddSymbol(uint32_t symbolId) {
        m_symbolsToUsers.emplace(symbolId, std::vector<crow::websocket::connection *>{});
    }

    void Broadcaster::ReportActions(std::vector<Data::OrderAction>& actions, uint32_t symbolId) {
        auto wsconn = m_symbolsToUsers.find(symbolId);
        if (wsconn == m_symbolsToUsers.end()) return;

        auto dto = VectorReturnable<OrderActionDto>(OrderActionDto::ToVector(actions), "events");
        auto json = dto.toJson();
        nlohmann::json payload = WsData::Payload {WsData::OpCodes::EVENTS, std::move(json)};
        std::string resp = payload.dump();

        for(auto* conn : wsconn->second)
        {
            conn->send_text(resp);
        }
    }

    void Broadcaster::OnLoginReguest(crow::websocket::connection& conn, const WsData::Payload& payload) {
        CORE_TRACE("WS RECEIVED LOGIN REQUEST");
        WsData::Login login = payload.payload.get<WsData::Login>();
        auto it = m_apiKeys.find(login.apikey);
        if (it == m_apiKeys.end()) {
            RespondError(conn, WsData::ErrorCodes::UNAUTHORIZED,
                         "ApiKey not found, please login Rest route before using the websocket");
            return;
        }
        auto userId = it->second;
        std::lock_guard<std::mutex> _(m_mtx);
        m_users[userId] = &conn;
        conn.userdata((void *) (userId));

        Ack(conn, WsData::OpCodes::READY);
    }

    void Broadcaster::OnSubscribeReguest(crow::websocket::connection& conn, const WsData::Payload& payload) {
        CORE_TRACE("WS RECEIVED SUBSCRIBE REQUEST");
        if (!IsLoggedIn(conn, m_users))
            return;

        if (IsRatelimited(conn, m_ratelimiter))
            return;

        WsData::SubSymbol sub = payload.payload.get<WsData::SubSymbol>();
        std::lock_guard<std::mutex> _(m_mtx);
        // check if sIds exist
        for (auto sid: sub.symbolIds) {
            auto sit = m_symbolsToUsers.find(sid);
            if (sit != m_symbolsToUsers.end())
                continue;

            RespondError(conn, WsData::ErrorCodes::NOT_FOUND,
                         "Symbol with Id " + std::to_string(sid) + " could not be found");
            return;
        }

        // Do the subscribing
        auto u = m_connections.find(&conn);
        for (auto sid: sub.symbolIds) {
            if (std::find(u->second.begin(), u->second.end(), sid) != u->second.end())
                continue;

            u->second.emplace_back(sid);
            // add to symbols list
            auto sit = m_symbolsToUsers.find(sid);
            sit->second.emplace_back(&conn);
        }

        Ack(conn, WsData::OpCodes::SUCCESS);
    }

    void Broadcaster::OnUnSubscribeReguest(crow::websocket::connection& conn, const WsData::Payload& payload) {
        CORE_TRACE("WS RECEIVED UNSUBSCRIBE REQUEST");
        if (!IsLoggedIn(conn, m_users))
            return;

        if (IsRatelimited(conn, m_ratelimiter))
            return;

        std::lock_guard<std::mutex> _(m_mtx);
        WsData::SubSymbol unsub = payload.payload.get<WsData::SubSymbol>();
        // check if sIds exist
        for (auto sid: unsub.symbolIds) {
            auto sit = m_symbolsToUsers.find(sid);
            if (sit != m_symbolsToUsers.end())
                continue;

            RespondError(conn, WsData::ErrorCodes::NOT_FOUND,
                         "Symbol with Id " + std::to_string(sid) + " could not be found");
            return;
        }

        // Do the unsubscribing
        auto u = m_connections.find(&conn);
        for (auto sid: unsub.symbolIds) {
            auto uit = std::find(u->second.begin(), u->second.end(), sid);
            if (uit == u->second.end())
                continue;

            uint32_t temp = u->second[u->second.size() - 1];
            u->second[u->second.size() - 1] = *uit;
            *uit = temp;
            u->second.pop_back();

            // remove ws from symbol list
            auto sit = m_symbolsToUsers.find(sid);
            auto wsconn = std::find(sit->second.begin(), sit->second.end(), &conn);

            auto wsend = --sit->second.end();
            crow::websocket::connection *wstemp = *wsend;
            *wsend = *wsconn;
            *wsconn = wstemp;
            sit->second.pop_back();
        }

        Ack(conn, WsData::OpCodes::SUCCESS);
    }

} // TradingEngine
