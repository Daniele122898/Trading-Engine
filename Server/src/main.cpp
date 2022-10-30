#include <log.h>
#include <MatchingEngine.h>
#include <crow.h>
#include "ThreadedLogReporter.h"

#include "dtos/SymbolDto.h"

using namespace TradingEngine;

int main() {
    Util::log::Init("Matching Engine");

    Matching::MatchingEngine<Matching::ThreadedLogOrderReporter> engine{
        Matching::MatchReporter(std::make_unique<Matching::ThreadedLogOrderReporter>())};

    Data::Symbol symbol{1, "AAPL"};
    engine.AddSymbol(symbol);

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
    CROW_ROUTE(app, "/symbol").methods("POST"_method)([](){
        return "Test";
    });
    CROW_ROUTE(app, "/symbols").methods("GET"_method)([](){
        return "Test";
    });
    CROW_ROUTE(app, "/symbol/<uint>").methods("GET"_method)([&engine](unsigned int symbolId){
        auto symbol = engine.Symbol(symbolId);
        return SymbolDto{symbol};
    });
    CROW_ROUTE(app, "/orderbook/<uint>").methods("GET"_method)([](unsigned int orderBookId){
        return "Test";
    });
    CROW_ROUTE(app, "/order").methods("POST"_method)([](){
        return "Test";
    });

    CORE_TRACE("Starting Server on port 18080");
    app.port(18080).multithreaded().run();
}
