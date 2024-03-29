//
// Created by danie on 11/28/2022.
//

#include "Broadcaster.h"
#include "dtos/WebsocketDto.h"
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

    void Broadcaster::ReportOrderCreation(const Data::Order &order) {
        m_creations.enqueue({order.Id, order.Price, order.SymbolId, order.InitialQuantity, order.Type, order.Side});
    }

    void
    Broadcaster::ReportOrderFill(const Data::Order &order, const Data::Order &counterOrder,
                                 Data::FillReason reason, uint32_t diff) {
        WsData::OpCodes opCode;
        switch (reason) {
            case Data::FillReason::SELF_TRADE:
                opCode = WsData::OpCodes::SELF_TRADE;
                break;
            case Data::FillReason::CANCELLED:
                opCode = WsData::OpCodes::CANCELLED;
                break;
            case Data::FillReason::FILLED:
                opCode = WsData::OpCodes::FILLED;
                break;
            case Data::FillReason::EXPIRED:
                opCode = WsData::OpCodes::EXPIRED;
                break;
        }

        m_reports.emplace(order.Id, counterOrder.Id, order.SymbolId, diff, opCode);
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

            /*Switch case entries work by jumps. If you jump across a variable initialization then you cannot be sure the stack is in an ok state because you may push something to the stack when you handle a case but if you leave the switch block, you have no way of tracking that additional push you made. And so any variables defined under a case need to be popped off the stack when you leave the switch block, and the only way to do that is to surround it in its own scope*/
            switch (request.opcode) {
                case WsData::OpCodes::LOGIN: {
                    CORE_TRACE("WS RECEIVED LOGIN REQUEST");
                    WsData::Login login = request.payload.get<WsData::Login>();
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
                    return;
                }
                case WsData::OpCodes::SUBSCRIBE: {
                    CORE_TRACE("WS RECEIVED SUBSCRIBE REQUEST");
                    if (!IsLoggedIn(conn, m_users))
                        return;

                    if (IsRatelimited(conn, m_ratelimiter))
                        return;

                    WsData::SubSymbol sub = request.payload.get<WsData::SubSymbol>();
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
                    return;
                }
                case WsData::OpCodes::UNSUBSCRIBE: {
                    CORE_TRACE("WS RECEIVED UNSUBSCRIBE REQUEST");
                    if (!IsLoggedIn(conn, m_users))
                        return;

                    if (IsRatelimited(conn, m_ratelimiter))
                        return;

                    std::lock_guard<std::mutex> _(m_mtx);
                    WsData::SubSymbol unsub = request.payload.get<WsData::SubSymbol>();
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

    void Broadcaster::BroadcastingLoop() {
        while (m_running) {
            WsData::Creation creation{};

            if (m_creations.try_dequeue(creation)) {
                auto wsconns = m_symbolsToUsers.find(creation.symbolId);
                nlohmann::json jdata = creation;
                nlohmann::json json = WsData::Payload {WsData::OpCodes::CREATION, std::move(jdata)};
                std::string resp = json.dump();
                for (auto conn: wsconns->second) {
                    conn->send_text(resp);
                }
            }

            WsData::ShareReport report{};
            // block wait for new reports, timed to be killable
            if (!m_reports.wait_dequeue_timed(report, std::chrono::milliseconds(1)))
                continue;

            auto sid = report.SymbolId;
            auto wsconns = m_symbolsToUsers.find(sid);

            std::string resp;

            if (report.OpCode == WsData::OpCodes::SELF_TRADE || report.OpCode == WsData::OpCodes::FILLED) {
                nlohmann::json j = WsData::ShareCounter{report.OrderId, report.CounterId, report.Diff};
                nlohmann::json json = WsData::Payload {report.OpCode, std::move(j)};
                resp = json.dump();
            } else {
                nlohmann::json j = WsData::Share{report.OrderId};
                nlohmann::json json = WsData::Payload {report.OpCode, std::move(j)};
                resp = json.dump();
            }

            for (auto conn: wsconns->second) {
                conn->send_text(resp);
            }
        }
    }

} // TradingEngine
