//
// Created by danie on 9/14/2022.
//

#ifndef TRADINGENGINE_MATCHINGENGINETEST_H
#define TRADINGENGINE_MATCHINGENGINETEST_H

#include <log.h>
#include <MatchingEngine.h>
#include "gtest/gtest.h"
#include <LogOrderReporter.h>

class MatchingEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        using namespace TradingEngine::Data;
        using namespace TradingEngine::Matching;

        Symbol symbol{1, "AAPL"};
        m_engine.AddSymbol(symbol);

        // fill out order book a little bit
        for (int i = 0; i < 10; ++i) {
            Order order{
                static_cast<uint64_t>(i),
                0,
                symbol.Id,
                OrderType::LIMIT,
                TradingEngine::Data::OrderSide::BUY,
                TradingEngine::Data::OrderLifetime::GFD,
                (i+1)*5,
                10};
            if (i >= 5)
                order.Side = TradingEngine::Data::OrderSide::SELL;

            m_engine.AddOrder(order);
        }

        CORE_INFO(*m_engine.OrderBook(symbol.Id));
    }

    TradingEngine::Matching::MatchingEngine<TradingEngine::Matching::LogOrderReporter> m_engine{TradingEngine::Matching::LogOrderReporter{}};

};

#endif //TRADINGENGINE_MATCHINGENGINETEST_H
