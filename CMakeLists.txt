cmake_minimum_required(VERSION 3.16.3)
project(TradingEngine)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(external/spdlog)
add_subdirectory(internal/data)

add_subdirectory(Server)
