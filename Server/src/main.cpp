#include <log.h>
#include <MatchingEngine.h>
#include <crow.h>
#include "ThreadedLogReporter.h"

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
    app.port(18080).multithreaded().run();
}
