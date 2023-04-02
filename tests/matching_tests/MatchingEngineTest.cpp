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
            1,
            TradingEngine::Data::OrderType::IOC,
            TradingEngine::Data::OrderSide::BUY,
            TradingEngine::Data::OrderLifetime::GFD,
            30,
            5
    };
    m_engine.AddOrder(order, m_actions);
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
            1,
            TradingEngine::Data::OrderType::IOC,
            TradingEngine::Data::OrderSide::BUY,
            TradingEngine::Data::OrderLifetime::GFD,
            20,
            5
    };
    m_engine.AddOrder(order, m_actions);
    auto volAfter = totalVol(asks);

    EXPECT_EQ(volBefore, volAfter);

    order= {
            13,
            1,
            1,
            TradingEngine::Data::OrderType::IOC,
            TradingEngine::Data::OrderSide::BUY,
            TradingEngine::Data::OrderLifetime::GFD,
            50,
            5
    };
    m_engine.AddOrder(order, m_actions);
    EXPECT_EQ(ob->BestAsk()->Price, 35);
}

TEST_F(MatchingEngineTest, TestIOCSell) {
    TradingEngine::Data::Order order {
            11,
            1,
            1,
            TradingEngine::Data::OrderType::IOC,
            TradingEngine::Data::OrderSide::SELL,
            TradingEngine::Data::OrderLifetime::GFD,
            25,
            5
    };
    m_engine.AddOrder(order, m_actions);
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
            1,
            TradingEngine::Data::OrderType::IOC,
            TradingEngine::Data::OrderSide::SELL,
            TradingEngine::Data::OrderLifetime::GFD,
            30,
            5
    };
    m_engine.AddOrder(order, m_actions);
    auto volAfter = totalVol(bids);

    EXPECT_EQ(volBefore, volAfter);

    order= {
            13,
            1,
            1,
            TradingEngine::Data::OrderType::IOC,
            TradingEngine::Data::OrderSide::SELL,
            TradingEngine::Data::OrderLifetime::GFD,
            10,
            5
    };
    m_engine.AddOrder(order, m_actions);
    EXPECT_EQ(ob->BestBid()->Price, 20);
}

TEST_F(MatchingEngineTest, TestFOKBuyFill) {
    TradingEngine::Data::Order order {
            11,
            1,
            1,
            TradingEngine::Data::OrderType::FOK,
            TradingEngine::Data::OrderSide::BUY,
            TradingEngine::Data::OrderLifetime::GFD,
            30,
            5
    };
    m_engine.AddOrder(order, m_actions);
    auto ob = m_engine.OrderBook(1);
    EXPECT_EQ(ob->BestAsk()->TotalVolume, 5);
}

TEST_F(MatchingEngineTest, TestFOKBuyFail) {
    TradingEngine::Data::Order order {
            11,
            1,
            1,
            TradingEngine::Data::OrderType::FOK,
            TradingEngine::Data::OrderSide::BUY,
            TradingEngine::Data::OrderLifetime::GFD,
            30,
            20
    };
    m_engine.AddOrder(order, m_actions);
    auto ob = m_engine.OrderBook(1);
    EXPECT_EQ(ob->BestAsk()->TotalVolume, 10);
}

TEST_F(MatchingEngineTest, TestFOKSellFill) {
    TradingEngine::Data::Order order {
            11,
            1,
            1,
            TradingEngine::Data::OrderType::FOK,
            TradingEngine::Data::OrderSide::SELL,
            TradingEngine::Data::OrderLifetime::GFD,
            25,
            5
    };
    m_engine.AddOrder(order, m_actions);
    auto ob = m_engine.OrderBook(1);
    EXPECT_EQ(ob->BestBid()->TotalVolume, 5);
}

TEST_F(MatchingEngineTest, TestFOKSellFail) {
    TradingEngine::Data::Order order {
            11,
            1,
            1,
            TradingEngine::Data::OrderType::FOK,
            TradingEngine::Data::OrderSide::SELL,
            TradingEngine::Data::OrderLifetime::GFD,
            25,
            20
    };
    m_engine.AddOrder(order, m_actions);
    auto ob = m_engine.OrderBook(1);
    EXPECT_EQ(ob->BestBid()->TotalVolume, 10);
}

TEST_F(MatchingEngineTest, TestMarketBuy) {
    TradingEngine::Data::Order order {
            11,
            1,
            1,
            TradingEngine::Data::OrderType::MARKET,
            TradingEngine::Data::OrderSide::BUY,
            TradingEngine::Data::OrderLifetime::GFD,
            30,
            5
    };
    m_engine.AddOrder(order, m_actions);
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
            1,
            TradingEngine::Data::OrderType::MARKET,
            TradingEngine::Data::OrderSide::BUY,
            TradingEngine::Data::OrderLifetime::GFD,
            20,
            5
    };
    m_engine.AddOrder(order, m_actions);
    auto volAfter = totalVol(asks);

    EXPECT_EQ(volBefore, volAfter+5);

    order= {
            13,
            1,
            1,
            TradingEngine::Data::OrderType::MARKET,
            TradingEngine::Data::OrderSide::BUY,
            TradingEngine::Data::OrderLifetime::GFD,
            50,
            5
    };
    m_engine.AddOrder(order, m_actions);
    EXPECT_EQ(ob->BestAsk()->Price, 35);
}

TEST_F(MatchingEngineTest, TestMarketSell) {
    TradingEngine::Data::Order order {
            11,
            1,
            1,
            TradingEngine::Data::OrderType::MARKET,
            TradingEngine::Data::OrderSide::SELL,
            TradingEngine::Data::OrderLifetime::GFD,
            25,
            5
    };
    m_engine.AddOrder(order, m_actions);
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
            1,
            TradingEngine::Data::OrderType::MARKET,
            TradingEngine::Data::OrderSide::SELL,
            TradingEngine::Data::OrderLifetime::GFD,
            30,
            5
    };
    m_engine.AddOrder(order, m_actions);
    auto volAfter = totalVol(bids);

    EXPECT_EQ(volBefore, volAfter+5);

    order= {
            13,
            1,
            1,
            TradingEngine::Data::OrderType::MARKET,
            TradingEngine::Data::OrderSide::SELL,
            TradingEngine::Data::OrderLifetime::GFD,
            10,
            5
    };
    m_engine.AddOrder(order, m_actions);
    EXPECT_EQ(ob->BestBid()->Price, 20);
}
