#include <log.h>
#include <MatchingEngine.h>

using namespace TradingEngine;

int main() {
    Util::log::Init("Matching Engine");

    Matching::MatchingEngine engine;
    Data::Order order(1,
                      1,
                      TradingEngine::Data::OrderType::MARKET,
                      TradingEngine::Data::OrderSide::BUY,
                      TradingEngine::Data::OrderLifetime::GFD,
                      10,
                      10);

    engine.AddOrder(std::move(order));
}
