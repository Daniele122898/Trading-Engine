#include <cstdint>
#include <log.h>
#include <crow.h>
#include <string>
#include <nlohmann/json.hpp>
#include "Ratelimiter.h"
#include "db.h"
#include "enc.h"

using namespace StatsEngine;

int main() {

    TradingEngine::Util::log::Init("Stats Engine");

    std::string dbConnectionString = "postgres://postgres:test123@localhost:5432/trading_test";
    Database db{dbConnectionString};

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

    CORE_TRACE("Starting Server on port 18080");
    app.port(18080).multithreaded().run();
}
