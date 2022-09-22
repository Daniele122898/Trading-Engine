#include <log.h>
#include <MatchingEngine.h>
#include "LogOrderReporter.h"

using namespace TradingEngine;

int main() {
    Util::log::Init("Matching Engine");

    Server::LogOrderReporter reporter{};

    Matching::MatchingEngine<Server::LogOrderReporter> engine{reporter};

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
}
