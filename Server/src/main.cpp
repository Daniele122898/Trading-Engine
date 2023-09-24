#include <cstdint>
#include <list>
#include <log.h>
#include <MatchingEngine.h>
#include <crow.h>
#include <chrono>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <mutex>

#include "ThreadedLogReporter.h"

#include "dtos/OrderActionDto.h"
#include "dtos/SymbolDto.h"
#include "dtos/VectorReturnable.h"
#include "dtos/OrderBookDto.h"

#include "db/db.h"
#include "Broadcaster.h"
#include "Ratelimiter.h"
#include "EODHandler.h"
#include "order.h"
#include "tm.h"

#include "enc.h"

using namespace TradingEngine;

namespace TradingEngine::Data {
    void from_json(nlohmann::json const &json, Order &order) {
//        json.at("id").get_to(order.Id);
//        json.at("userId").get_to(order.UserId);
        json.at("symbolId").get_to(order.SymbolId);
        json.at("type").get_to(order.Type);
        json.at("side").get_to(order.Side);
        json.at("lifetime").get_to(order.Lifetime);
        json.at("price").get_to(order.Price);
        json.at("initialQ").get_to(order.InitialQuantity);
        json.at("initialQ").get_to(order.CurrentQuantity);

        if (order.Lifetime == OrderLifetime::GTD) {
            auto days = json.at("daysToExpiry").get<int32_t>();
            if (days <= 0) {
                throw nlohmann::json::parse_error::create(105, 0, "Invalid amount of days", &json);
            } else {
                auto currently = std::chrono::time_point_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now()
                );
                currently += std::chrono::days(days);
                order.ExpiryMs = currently.time_since_epoch();
//                std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(currently)};
            }
        }

    }
}

int main() {

    Util::log::Init("Matching Engine");


    std::string dbConnectionString = "postgres://postgres:test123@localhost:5432/trading_test";
    TradingEngine::Db::Database db{dbConnectionString};
    db.CreateTablesIfNotExist();

    //    uint64_t lastOrderId = db.LastUsedOrderId();
    //    CORE_TRACE("Last used order ID: {}", lastOrderId);

    //    std::atomic<uint64_t> nextOrderId{++lastOrderId};
    std::unordered_map<std::string, uint64_t> users{};

    auto ratelimiter = Util::Ratelimiter();

    auto broadcaster = std::make_shared<Broadcaster>(users, ratelimiter);

//        Matching::MatchingEngine<Matching::ThreadedLogOrderReporter, Db::FillPersistence, Broadcaster> engine{
//                Matching::MatchReporter(
//                        std::make_unique<Matching::ThreadedLogOrderReporter>(),
//                        std::make_unique<Db::FillPersistence>(db),
//                                broadcaster)};

    Data::OrderManager orderManager{};
    Matching::MatchingEngine engine{orderManager};

    auto symbols = db.GetSymbols();
    for (auto &symb: symbols) {
        CORE_TRACE("Adding symbol {} {}", symb.Id, symb.Ticker);
        orderManager.AddSymbol(symb);
        broadcaster->AddSymbol(symb.Id);
    }

    auto eodhandler = EODHandler(dbConnectionString, orderManager);

    uint64_t nextOrderId = db.LargestFillId() + 1;

    std::mutex mtx;
    crow::SimpleApp app;

#ifndef NDEBUG
    app.loglevel(crow::LogLevel::Debug);
#else
    app.loglevel(crow::LogLevel::Warning);
#define CROW_ENFORCE_WS_SPEC
#endif
#define AUTHENTICATE(req) std::string apikey = req.get_header_value("Authorization"); \
        auto it = users.find(apikey); \
        if (it == users.end()) { \
            return crow::response{crow::UNAUTHORIZED}; \
        }\
        auto userId = it->second

#define EMPTY_BODY(req) if (req.body.empty()) { \
        return crow::response{400}; \
    }

#define RATELIMITED(routeType, userId) if (ratelimiter.IsRatelimited(routeType, userId)) { \
        return crow::response{crow::status::TOO_MANY_REQUESTS};\
    }

    CROW_ROUTE(app, "/ws")
            .websocket()
            .onaccept([](const crow::request &req) {
                return true;
            })
            .onopen([&broadcaster](crow::websocket::connection &conn) {
                broadcaster->OnOpen(conn);
            })
            .onclose([&broadcaster](crow::websocket::connection &conn, const std::string &reason) {
                broadcaster->OnClose(conn, reason);
            })
            .onmessage([&broadcaster](crow::websocket::connection &conn, const std::string &data, bool is_binary) {
                broadcaster->OnMessage(conn, data, is_binary);
            })
            .onerror([&broadcaster](crow::websocket::connection &conn) {
                broadcaster->OnError(conn);
            });

    CROW_ROUTE(app, "/login").methods("POST"_method)([&db, &users](const crow::request &req) {
        EMPTY_BODY(req);

        try {
            auto json = nlohmann::json::parse(req.body);

            if (json.contains("apikey")) {
                auto apikey = json.at("apikey").get<std::string>();
                uint64_t userId;
                if (!db.TryGetUserId(apikey, userId)) {
                    return crow::response{crow::BAD_REQUEST};
                }
                users[apikey] = userId;
            } else {
                auto username = json.at("username").get<std::string>();
                auto password = json.at("password").get<std::string>();

                uint64_t userId;
                std::basic_string<std::byte> passhash;
                std::basic_string<std::byte> salt;
                std::string apikey;
                if (!db.TryGetUser(username, userId, passhash, salt, apikey)) {
                    return crow::response{crow::BAD_REQUEST};
                }
                if (!sha256_match(passhash, salt, password)) {
                    return crow::response{crow::BAD_REQUEST};
                }

                users[apikey] = userId;

                return crow::response{apikey};
            }
        }
        catch (pqxx::sql_error const &e) {
            CORE_ERROR("SQL ERROR: {}", e.what());
            CORE_ERROR("QUERY: {}", e.query());
            return crow::response{crow::status::BAD_REQUEST, "Misformed query"};
        }
        catch (nlohmann::json::parse_error &ex) {
            CORE_ERROR("FAILED TO PARSE JSON {}\n at byte {}\n{}", req.body, ex.byte, ex.what());
            return crow::response{crow::status::BAD_REQUEST, "Bad JSON"};
        }
        catch (std::exception const &e) {
            CORE_ERROR("ERROR: {}", e.what());
            return crow::response{crow::status::BAD_REQUEST};
        }

        return crow::response{crow::OK};
    });

    // Endpoints
    CROW_ROUTE(app, "/register").methods("POST"_method)([&db](const crow::request &req) {
        EMPTY_BODY(req);

        try {
            auto json = nlohmann::json::parse(req.body);

            auto username = json.at("username").get<std::string>();
            auto password = json.at("password").get<std::string>();
            auto email = json.at("email").get<std::string>();

            unsigned char hash[32];
            unsigned char salt[32];
            unsigned char apikey[32];
            sha256_salted(password, hash, salt);
            get_rand(apikey, 32);
            auto userid = db.AddUser(username, email, hash, salt, apikey);
        }
        catch (pqxx::sql_error const &e) {
            CORE_ERROR("SQL ERROR: {}", e.what());
            CORE_ERROR("QUERY: {}", e.query());
            return crow::response{crow::status::BAD_REQUEST, "Misformed query"};
        }
        catch (nlohmann::json::parse_error &ex) {
            CORE_ERROR("FAILED TO PARSE JSON {}\n at byte {}\n{}", req.body, ex.byte, ex.what());
            return crow::response{crow::status::BAD_REQUEST, "Bad JSON"};
        }
        catch (std::exception const &e) {
            CORE_ERROR("ERROR: {}", e.what());
            return crow::response{crow::status::BAD_REQUEST};
        }

        return crow::response{crow::OK};
    });

    // TODO LIMIT TO ONLY ADMINS
    CROW_ROUTE(app, "/symbol").methods("POST"_method)([&orderManager, &db, &users, &broadcaster](const crow::request &req) {
        AUTHENTICATE(req);
        EMPTY_BODY(req);

        if (userId != 1) {
            return crow::response(crow::status::FORBIDDEN);
        }

        try {
            auto json = nlohmann::json::parse(req.body);

            auto id = db.AddSymbol(json["ticker"]);
            CORE_TRACE("Adding symbol {} with id {}", json["ticker"], id);

            Data::Symbol symbol{id, json["ticker"]};
            orderManager.AddSymbol(symbol);
            broadcaster->AddSymbol(symbol.Id);
        }
        catch (pqxx::sql_error const &e) {
            CORE_ERROR("SQL ERROR: {}", e.what());
            CORE_ERROR("QUERY: {}", e.query());
            return crow::response{crow::status::BAD_REQUEST, "Misformed or already existing ticker name"};
        }
        catch (nlohmann::json::parse_error &ex) {
            CORE_ERROR("FAILED TO PARSE JSON {}\n at byte {}\n{}", req.body, ex.byte, ex.what());
            return crow::response{crow::status::BAD_REQUEST, "Bad JSON"};
        }
        catch (std::exception const &e) {
            CORE_ERROR("ERROR: {}", e.what());
            return crow::response{crow::status::BAD_REQUEST};
        }

        return crow::response{crow::status::OK};
    });

    CROW_ROUTE(app, "/symbols").methods("GET"_method)([&orderManager, &users, &ratelimiter](const crow::request &req) {
        AUTHENTICATE(req);
        RATELIMITED(Util::BUCKET_TYPE::LISTS, userId);

        auto symbols = orderManager.GetSymbols();
        std::vector<SymbolDto> symbolsDto{};
        symbolsDto.reserve(symbols.size());

        for (auto &s: symbols) {
            SymbolDto sdto{std::move(s)};
            symbolsDto.push_back(sdto);
        }

        return crow::response{VectorReturnable{std::move(symbolsDto), "symbols"}};
    });

    CROW_ROUTE(app, "/symbol/<uint>").methods("GET"_method)(
            [&orderManager, &users, &ratelimiter](const crow::request &req, unsigned int symbolId) {
                AUTHENTICATE(req);
                RATELIMITED(Util::BUCKET_TYPE::SIMPLE, userId);

                auto symbol = orderManager.GetSymbol(symbolId);
                if (symbol == nullptr) {
                    return crow::response{404};
                }
                return crow::response{SymbolDto{*symbol}};
            });

    CROW_ROUTE(app, "/orderbook/<uint>").methods("GET"_method)(
            [&orderManager, &users, &ratelimiter](const crow::request &req, unsigned int orderBookId) {
                AUTHENTICATE(req);
                RATELIMITED(Util::BUCKET_TYPE::ORDER_BOOK, userId);

                Data::OrderBook const *book = orderManager.GetOrderBook(orderBookId);
                if (book == nullptr) {
                    return crow::response{404};
                }
                return crow::response{OrderBookDto{book}};
            });


    CROW_ROUTE(app, "/order/<uint>").methods("DELETE"_method)(
            [&orderManager, &users, &ratelimiter, &mtx](const crow::request &req, unsigned int orderId) {
                AUTHENTICATE(req);
                RATELIMITED(Util::BUCKET_TYPE::SIMPLE, userId);

                {
                    std::lock_guard<std::mutex> _(mtx);
                    // check if order exists and belongs to us
                    auto order = orderManager.FindOrder(orderId);
                    if (!order) {
                        return crow::response(404, "Order Not Found");
                    }
                    if (order->UserId != userId) {
                        return crow::response(crow::status::FORBIDDEN, "Not your order");
                    }

                    // remove order
                    orderManager.RemoveOrder(orderId);
                    // FIXME: Make sure this is re-implemented
    //                engine.CreateReport(order.value(), Data::Action::CANCELLED);
                }

                return crow::response{crow::status::OK};
            });

    CROW_ROUTE(app, "/order").methods("POST"_method)(
            [&engine,  &users, &broadcaster, &ratelimiter, &mtx, &nextOrderId, &db]
                    (const crow::request &req) {
                AUTHENTICATE(req);
                EMPTY_BODY(req);
                RATELIMITED(Util::BUCKET_TYPE::SIMPLE, userId);

                // Check if end of trading day
                auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now()
                );
                const auto eod = Util::GetPointInToday(now, 23, 0, 0);
                if (now >= eod) {
                    // stop trading
                    return crow::response{425, "End of Trading day. Restarts at beginning of next day."};
                }

                try {
                    auto json = nlohmann::json::parse(req.body);
                    auto order = json.get<Data::Order>();
                    order.UserId = userId;
                    std::vector<Data::OrderAction> actions{};

                    // FIXME: Make sure creation is BEFORE fill, remove fill from matching engine
                    {
                        std::lock_guard<std::mutex> _(mtx);

                        order.CreationTp = now;
                        order.Id = nextOrderId++;

                        if (order.Type == Data::OrderType::LIMIT) {
                            actions.emplace_back(Data::Action::CREATION, order, std::nullopt, 0);
                        }

                        CORE_TRACE("INSERTED ORDER WITH ID {}", order.Id);
                        engine.AddOrder(order, actions);
                    }

                    if (actions.empty()) {
                        // FIXME: Better error response
                        return crow::response(crow::status::BAD_REQUEST, "Failed to create order");
                    }
                    // TODO: Jsonifiyng data twice, for broadcast then private. maybe do it once?
                    broadcaster->ReportActions(actions, order.SymbolId);

                    // TODO: Add persistance + dropcopy
                    // TODO: Don't create stream if no action is a fill!!!!!
                    db.ProcessActions(order, actions);

                    // FIXME: Fix this temporary mess
                    return crow::response{VectorReturnable<OrderActionDto>(OrderActionDto::ToVector(actions), "events")};
                }
                catch (pqxx::sql_error const &e) {
                    CORE_ERROR("SQL ERROR: {}", e.what());
                    CORE_ERROR("QUERY: {}", e.query());
                    return crow::response{crow::status::BAD_REQUEST, "Misformed order entry"};
                }
                catch (nlohmann::json::parse_error &ex) {
                    CORE_ERROR("FAILED TO PARSE JSON {}\n at byte {}\n{}", req.body, ex.byte, ex.what());
                    return crow::response{crow::status::BAD_REQUEST, "Bad JSON"};
                }
                catch (std::exception const &e) {
                    CORE_ERROR("ERROR: {}", e.what());
                    return crow::response{crow::status::BAD_REQUEST};
                }

            });

    CORE_TRACE("Starting Server on port 18080");
    app.port(18080).multithreaded().run();
}

