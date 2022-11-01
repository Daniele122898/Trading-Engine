#include <log.h>
#include <MatchingEngine.h>
#include <crow.h>
#include "ThreadedLogReporter.h"

#include "dtos/SymbolDto.h"
#include "dtos/VectorReturnable.h"

using namespace TradingEngine;

int main() {
    Util::log::Init("Matching Engine");

    Matching::MatchingEngine<Matching::ThreadedLogOrderReporter> engine{
            Matching::MatchReporter(std::make_unique<Matching::ThreadedLogOrderReporter>())};

    uint32_t sId = 1;

    Data::Symbol symbol{sId++, "AAPL"};
    Data::Symbol symbol2{sId++, "GOOG"};
    engine.AddSymbol(symbol);
    engine.AddSymbol(symbol2);

    Data::Order order(1,
                      1,
                      1,
                      TradingEngine::Data::OrderType::LIMIT,
                      TradingEngine::Data::OrderSide::BUY,
                      TradingEngine::Data::OrderLifetime::GFD,
                      10,
                      10);

    engine.AddOrder(order);

    crow::SimpleApp app;

#ifndef NDEBUG
    app.loglevel(crow::LogLevel::Debug);
#else
    app.loglevel(crow::LogLevel::Warning);
#endif

    // Endpoints
    CROW_ROUTE(app, "/symbol").methods("POST"_method)([&engine, &sId](const crow::request &req) {
        auto json = nlohmann::json::parse(req.body);
        Data::Symbol symbol{sId++, json["ticker"]};
        engine.AddSymbol(symbol);

        return crow::response{200};
    });
    CROW_ROUTE(app, "/symbols").methods("GET"_method)([&engine]() {
        auto symbols = engine.Symbols();
        std::vector<SymbolDto> symbolsDto{};
        symbolsDto.reserve(symbols.size());

        for (auto& s:symbols) {
            SymbolDto sdto{std::move(s)};
            symbolsDto.push_back(sdto);
        }

        return VectorReturnable{std::move(symbolsDto), "symbols"};

    });
    CROW_ROUTE(app, "/symbol/<uint>").methods("GET"_method)([&engine](unsigned int symbolId) {
        auto symbol = engine.Symbol(symbolId);
        return SymbolDto{*symbol};
    });
    CROW_ROUTE(app, "/orderbook/<uint>").methods("GET"_method)([](unsigned int orderBookId) {
        return "Test";
    });
    CROW_ROUTE(app, "/order").methods("POST"_method)([]() {
        return "Test";
    });

    CORE_TRACE("Starting Server on port 18080");
    app.port(18080).multithreaded().run();
}
