//
// Created by danie on 9/14/2022.
//

#include "gtest/gtest.h"

#include <log.h>

int main(int argc, char **argv) {
    TradingEngine::Util::log::Init("TEST");

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}