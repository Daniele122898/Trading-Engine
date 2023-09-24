//
// Created by danie on 9/17/2023.
//

#include "OrderManager.h"
#include "log.h"

namespace TradingEngine::Data {

    std::optional<Data::Order> OrderManager::FindOrder(uint64_t id) {
        auto it = mOrders.find(id);
        if (it == mOrders.end()) {
            return std::nullopt;
        }

        return it->second;
    }

    Data::Order OrderManager::RemoveOrder(uint64_t id) {
        auto it = mOrders.find(id);
        if (it == mOrders.end())
            throw std::runtime_error("Couldn't find Order with ID: " + std::to_string(id));

        auto order = it->second;
        auto obIt = mOrderBooks.find(order.SymbolId);
        obIt->second.RemoveOrder(order.Id);
        mOrders.erase(order.Id);
        return order;
    }

    void OrderManager::AddNonMatchOrder(Data::Order &order) {
        CORE_TRACE("Adding Order: {}", order);
        auto obIt = mOrderBooks.find(order.SymbolId);
        if (obIt == mOrderBooks.end()) {
            return;
        }

        auto &ob = obIt->second;

        // Order has not been fully filled, thus place it.
        mOrders[order.Id] = order;
        ob.AddOrder(mOrders.at(order.Id));
    }

    void OrderManager::AddOrder(Data::Order &order, std::vector<Data::OrderAction> &actions) {
        CORE_TRACE("Received Order: {}", order);
        auto obIt = mOrderBooks.find(order.SymbolId);
        if (obIt == mOrderBooks.end()) {
            return;
        }

        auto &ob = obIt->second;

        // Order has not been fully filled, thus place it.
        mOrders[order.Id] = order;
        ob.AddOrder(mOrders.at(order.Id));
    }

    void OrderManager::AddSymbol(const Data::Symbol &symbol) {
        // check if symbol already exists
        if (mSymbols.contains(symbol.Id))
            return;

        mSymbols[symbol.Id] = symbol;
        // create orderbook
        mOrderBooks.insert({symbol.Id, Data::OrderBook{symbol}});
    }

    Data::OrderBook* OrderManager::GetOrderBook(uint32_t symbolId) {
        auto res = mOrderBooks.find(symbolId);
        if (res == mOrderBooks.end()) {
            return nullptr;
        }

        return &(res->second);
    }

    std::vector<Data::Symbol> OrderManager::GetSymbols() {
        std::vector<Data::Symbol> symbols{mSymbols.size()};
        int i = 0;
        for (auto it = mSymbols.begin(); it != mSymbols.end(); ++it, ++i) {
            symbols.at(i) = it->second;
        }

        return symbols;
    }

    Data::Symbol const *OrderManager::GetSymbol(uint32_t id) {
        auto it = mSymbols.find(id);
        if (it == mSymbols.end())
            return nullptr;

        return &(it->second);
    }

    std::vector<Data::Order> OrderManager::RemoveExpiredOrders(std::chrono::system_clock::time_point now) {
        std::vector<Data::Order> expiredOrders{};

        for (auto& [id, order] : mOrders) {
            switch (order.Lifetime) {
                case OrderLifetime::GFD:
                    expiredOrders.push_back(order);
                    continue;
                case OrderLifetime::GTD:
                {
                    auto daysNow = std::chrono::floor<std::chrono::days>(now);

                    std::chrono::system_clock::time_point expiry {order.ExpiryMs};
                    auto daysExpiry = std::chrono::floor<std::chrono::days>(expiry);


                    if (daysExpiry <= daysNow) {
                        expiredOrders.push_back(order);
                    }
                    continue;
                }
                case OrderLifetime::GTC:
                    continue;
            }

        }

        for (auto& order : expiredOrders) {
            RemoveOrder(order.Id);
        }

        return expiredOrders;
    }
}