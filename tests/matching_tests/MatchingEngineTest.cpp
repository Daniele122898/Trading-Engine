//
// Created by danie on 9/14/2022.
//

#include "MatchingEngineTest.h"

TEST_F(MatchingEngineTest, ProperSetup) {
    auto symbols = m_engine.Symbols();
    EXPECT_EQ(symbols.size(), 1);

    auto book = m_engine.OrderBook(symbols[0].Id);
    ASSERT_NE(book, nullptr);

    EXPECT_EQ(book->BestBid()->Price, 25);
    EXPECT_EQ(book->BestAsk()->Price, 30);
}

TEST_F(MatchingEngineTest, TestIOCBuy) {
    TradingEngine::Data::Order order {
            11,
            1,
            TradingEngine::Data::OrderType::IOC,
            TradingEngine::Data::OrderSide::BUY,
            TradingEngine::Data::OrderLifetime::GFD,
            30,
            5
    };
    m_engine.AddOrder(order);
    auto ob = m_engine.OrderBook(1);
    EXPECT_EQ(ob->BestAsk()->TotalVolume, 5);

    auto& asks = ob->Asks();
    auto totalVol = [](auto& s) -> uint64_t {
        uint64_t vol = 0;
        for (auto& lvl:s) {
            vol += lvl.TotalVolume;
        }
        return vol;
    };
    auto volBefore = totalVol(asks);
    order = {
            12,
            1,
            TradingEngine::Data::OrderType::IOC,
            TradingEngine::Data::OrderSide::BUY,
            TradingEngine::Data::OrderLifetime::GFD,
            20,
            5
    };
    m_engine.AddOrder(order);
    auto volAfter = totalVol(asks);

    EXPECT_EQ(volBefore, volAfter);

    order= {
            13,
            1,
            TradingEngine::Data::OrderType::IOC,
            TradingEngine::Data::OrderSide::BUY,
            TradingEngine::Data::OrderLifetime::GFD,
            50,
            5
    };
    m_engine.AddOrder(order);
    EXPECT_EQ(ob->BestAsk()->Price, 35);
}

TEST_F(MatchingEngineTest, TestIOCSell) {
    TradingEngine::Data::Order order {
            11,
            1,
            TradingEngine::Data::OrderType::IOC,
            TradingEngine::Data::OrderSide::SELL,
            TradingEngine::Data::OrderLifetime::GFD,
            25,
            5
    };
    m_engine.AddOrder(order);
    auto ob = m_engine.OrderBook(1);
    EXPECT_EQ(ob->BestBid()->TotalVolume, 5);

    auto& bids = ob->Bids();
    auto totalVol = [](auto& s) -> uint64_t {
        uint64_t vol = 0;
        for (auto& lvl:s) {
            vol += lvl.TotalVolume;
        }
        return vol;
    };
    auto volBefore = totalVol(bids);
    order = {
            12,
            1,
            TradingEngine::Data::OrderType::IOC,
            TradingEngine::Data::OrderSide::SELL,
            TradingEngine::Data::OrderLifetime::GFD,
            30,
            5
    };
    m_engine.AddOrder(order);
    auto volAfter = totalVol(bids);

    EXPECT_EQ(volBefore, volAfter);

    order= {
            13,
            1,
            TradingEngine::Data::OrderType::IOC,
            TradingEngine::Data::OrderSide::SELL,
            TradingEngine::Data::OrderLifetime::GFD,
            10,
            5
    };
    m_engine.AddOrder(order);
    EXPECT_EQ(ob->BestBid()->Price, 20);
}
