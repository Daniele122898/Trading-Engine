//
// Created by danie on 9/17/2023.
//
#pragma once

#include <functional>
#include <unordered_map>
#include <cstdint>
#include "symbol.h"
#include "order_book.h"
#include "order.h"

namespace TradingEngine::Data {

    class OrderManager {

    public:
        std::optional<Data::Order> FindOrder(uint64_t id);
        Data::Order RemoveOrder(uint64_t id);
        void AddNonMatchOrder(Data::Order& order);
        void AddOrder(Data::Order &order, std::vector<Data::OrderAction>& actions);
        void AddSymbol(const Data::Symbol & symbol);
        Data::OrderBook *GetOrderBook(uint32_t symbolId);
        std::vector<Data::Symbol> GetSymbols();
        Data::Symbol const *GetSymbol(uint32_t id);
        std::vector<Data::Order> RemoveExpiredOrders(std::chrono::system_clock::time_point now);

    private:

        std::unordered_map<uint32_t, Data::Symbol> mSymbols{};

        std::unordered_map<uint32_t, Data::OrderBook> mOrderBooks{};
        std::unordered_map<uint64_t, Data::Order> mOrders{};
    };

}
