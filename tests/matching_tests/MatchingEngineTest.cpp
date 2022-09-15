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
