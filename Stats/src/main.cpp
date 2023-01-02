#include <cstdint>
#include <log.h>
#include <crow.h>
#include <string>

using namespace TradingEngine;

int main() {

    Util::log::Init("Stats Engine");
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

    CORE_TRACE("Starting Server on port 18080");
    app.port(18080).multithreaded().run();
}

