#include <chrono>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <log.h>
#include <crow.h>
#include <sstream>
#include <string>
#include <nlohmann/json.hpp>
#include "Ratelimiter.h"
#include "db/engineDb.h"
#include "db/statsDb.h"
#include "enc.h"
#include "symbol.h"

using namespace StatsEngine;


std::tm GetNextInterval(std::tm&  start) {
    // TODO: Check the rigorousness of this timezone approach
    auto tp = std::chrono::system_clock::from_time_t(std::mktime(&start) - timezone); 
    tp += std::chrono::minutes(5);
    auto ett = std::chrono::system_clock::to_time_t(tp);
    std::tm etm = *std::gmtime(&ett);
    return etm;
}

int main() {

    TradingEngine::Util::log::Init("Stats Engine");

    std::string engineDbConnStr = "postgres://postgres:test123@localhost:5432/trading_test";
    Db::EngineDb engineDb{engineDbConnStr};

    std::string statsDbConnStr = "postgres://postgres:test123@localhost:5432/stats_test";
    Db::StatsDb statsDb{statsDbConnStr, engineDbConnStr};
    statsDb.CreateTablesIfNotExist();
    auto tm = statsDb.GetFirstTimestamp(1);
    if (tm.tm_min % 10 >= 5) {
        tm.tm_min -= (tm.tm_min % 10) - 5;
        tm.tm_sec = 0;
    } else {
        tm.tm_min -= tm.tm_min % 10;
        tm.tm_sec = 0;
    }
    // get Starttime
    std::ostringstream st{};
    st << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

    // get endtime
    std::ostringstream et{};
    auto etm = GetNextInterval(tm);
    et << std::put_time(&etm, "%Y-%m-%d %H:%M:%S");
    CORE_TRACE("TIME INTERVAL: {} - {}", st.str(), et.str());
    return 0;

    auto symbols = engineDb.GetSymbols();
    // std::unordered_map<uint32_t, TradingEngine::Data::Symbol> sym{};
    // for (auto &symb: symbols) {
    //     CORE_TRACE("Adding symbol {} {}", symb.Id, symb.Ticker);
    //     sym.emplace(symb.Id, symb);
    // }

    auto ratelimiter = TradingEngine::Util::Ratelimiter();
    std::unordered_map<std::string, uint64_t> users{};

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

    CROW_ROUTE(app, "/login").methods("POST"_method)([&engineDb, &users](const crow::request &req) {
        EMPTY_BODY(req);

        try {
            auto json = nlohmann::json::parse(req.body);

            if (json.contains("apikey")) {
                auto apikey = json.at("apikey").get<std::string>();
                uint64_t userId;
                if (!engineDb.TryGetUserId(apikey, userId)) {
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
                if (!engineDb.TryGetUser(username, userId, passhash, salt, apikey)) {
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
    CROW_ROUTE(app, "/register").methods("POST"_method)([&engineDb](const crow::request &req) {
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
            auto userid = engineDb.AddUser(username, email, hash, salt, apikey);
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

    CORE_TRACE("Starting Server on port 18080");
    app.port(18090).multithreaded().run();
}
