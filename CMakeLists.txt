cmake_minimum_required(VERSION 3.16.3)
project(TradingEngine)

set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(external/spdlog)
add_subdirectory(external/gtest)
add_subdirectory(external/json)
add_subdirectory(external/libpqxx)
add_subdirectory(external/rwq)

add_subdirectory(internal/data)
add_subdirectory(internal/util)
add_subdirectory(internal/matching)

add_subdirectory(Server)
add_subdirectory(tests)
