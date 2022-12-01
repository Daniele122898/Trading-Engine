#include <log.h>
#include <MatchingEngine.h>
#include <crow.h>
#include <chrono>
#include <iostream>
#include <string>
#include <unordered_map>

#include "ThreadedLogReporter.h"

#include "dtos/SymbolDto.h"
#include "dtos/VectorReturnable.h"
#include "dtos/OrderBookDto.h"

#include "db/db.h"
#include "db/fill_persistence.h"
#include "Broadcaster.h"

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
            if (days == 1) {
                order.Lifetime = OrderLifetime::GFD;
            } else if (days <= 0) {
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

//    std::chrono::time_point currently = std::chrono::time_point_cast<std::chrono::milliseconds>(
//            std::chrono::system_clock::now()
//    );
//    std::chrono::duration millis_since_utc_epoch = currently.time_since_epoch();

    Util::log::Init("Matching Engine");

    std::chrono::time_point currently = std::chrono::time_point_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now()
    );
    std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp{std::chrono::seconds{1669039514}};
    auto days = std::chrono::floor<std::chrono::days>(tp);

    std::chrono::year_month_day ymd{days};
    std::cout << "Current Year: " << static_cast<int>(ymd.year())
              << ", Month: " << static_cast<unsigned>(ymd.month())
              << ", Day: " << static_cast<unsigned>(ymd.day()) << '\n';
    std::cout << std::chrono::system_clock::to_time_t(currently) << std::endl;
    currently += std::chrono::days(10);

    ymd = std::chrono::floor<std::chrono::days>(currently);
    std::cout << "Current Year: " << static_cast<int>(ymd.year())
              << ", Month: " << static_cast<unsigned>(ymd.month())
              << ", Day: " << static_cast<unsigned>(ymd.day()) << '\n';

    std::cout << std::chrono::system_clock::to_time_t(currently) << std::endl;
    std::chrono::duration millis_since_utc_epoch = currently.time_since_epoch();

    TradingEngine::Db::Database db{"postgres://postgres:test123@localhost:5432/trading_test"};
    db.CreateTablesIfNotExist();

//    uint64_t lastOrderId = db.LastUsedOrderId();
//    CORE_TRACE("Last used order ID: {}", lastOrderId);

//    std::atomic<uint64_t> nextOrderId{++lastOrderId};
    std::unordered_map<std::string, uint64_t> users{};

    auto broadcaster = std::make_shared<Broadcaster>(users);

    Matching::MatchingEngine<Matching::ThreadedLogOrderReporter, Db::FillPersistence, Broadcaster> engine{
            Matching::MatchReporter(
                    std::make_unique<Matching::ThreadedLogOrderReporter>(),
                    std::make_unique<Db::FillPersistence>(db),
                            broadcaster)};


    auto symbols = db.GetSymbols();
    for (auto &symb: symbols) {
        CORE_TRACE("Adding symbol {} {}", symb.Id, symb.Ticker);
        engine.AddSymbol(symb);
        broadcaster->AddSymbol(symb.Id);
    }

    // TODO LOAD ORDERS ON STARTUP


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

    CROW_ROUTE(app, "/ws")
        .websocket()
        .onaccept([](const crow::request& req){
            return true;
        })
        .onopen([&broadcaster](crow::websocket::connection& conn) {
            broadcaster->OnOpen(conn);
        })
        .onclose([&broadcaster](crow::websocket::connection& conn, const std::string& reason) {
            broadcaster->OnClose(conn, reason);
        })
        .onmessage([&broadcaster](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
            broadcaster->OnMessage(conn, data, is_binary);
        })
        .onerror([&broadcaster](crow::websocket::connection& conn) {
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

    CROW_ROUTE(app, "/symbol").methods("POST"_method)([&engine, &db, &users, &broadcaster](const crow::request &req) {
        AUTHENTICATE(req);
        EMPTY_BODY(req);

        try {
            auto json = nlohmann::json::parse(req.body);

            auto id = db.AddSymbol(json["ticker"]);
            CORE_TRACE("Adding symbol {} with id {}", json["ticker"], id);

            Data::Symbol symbol{id, json["ticker"]};
            engine.AddSymbol(symbol);
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

    CROW_ROUTE(app, "/symbols").methods("GET"_method)([&engine, &users](const crow::request &req) {
        AUTHENTICATE(req);

        auto symbols = engine.Symbols();
        std::vector<SymbolDto> symbolsDto{};
        symbolsDto.reserve(symbols.size());

        for (auto &s: symbols) {
            SymbolDto sdto{std::move(s)};
            symbolsDto.push_back(sdto);
        }

        return crow::response{VectorReturnable{std::move(symbolsDto), "symbols"}};
    });

    CROW_ROUTE(app, "/symbol/<uint>").methods("GET"_method)([&engine, &users](const crow::request &req, unsigned int symbolId) {
        AUTHENTICATE(req);

        auto symbol = engine.Symbol(symbolId);
        if (symbol == nullptr) {
            return crow::response{404};
        }
        return crow::response{SymbolDto{*symbol}};
    });

    CROW_ROUTE(app, "/orderbook/<uint>").methods("GET"_method)([&engine, &users](const crow::request &req, unsigned int orderBookId) {
        AUTHENTICATE(req);

        Data::OrderBook const *book = engine.OrderBook(orderBookId);
        if (book == nullptr) {
            return crow::response{404};
        }
        return crow::response{OrderBookDto{book}};
    });

    CROW_ROUTE(app, "/order").methods("POST"_method)([&engine, &db, &users, &broadcaster](const crow::request &req) {
        AUTHENTICATE(req);
        EMPTY_BODY(req);

        try {
            auto json = nlohmann::json::parse(req.body);
            auto order = json.get<Data::Order>();
            order.CreationTp = std::chrono::time_point_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now()
            );
            // TODO move to seperate thread
            auto id = db.AddOrder(order);
            order.Id = id;
            order.UserId = userId;
            CORE_TRACE("ADDED ORDER WITH ID {}", id);
//            nextOrderId.store(++id);
            broadcaster->ReportOrderCreation(order);
            engine.AddOrder(order);
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

        return crow::response{crow::status::OK};
    });

    CORE_TRACE("Starting Server on port 18080");
    app.port(18080).multithreaded().run();
}

