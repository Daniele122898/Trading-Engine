//
// Created by danie on 9/17/2023.
//

#include "OrderManager.h"
#include "log.h"

namespace TradingEngine::Data {

    std::optional<Data::Order> OrderManager::FindOrder(uint64_t id) {
        auto it = m_orders.find(id);
        if (it == m_orders.end()) {
            return std::nullopt;
        }

        return it->second;
    }

    Data::Order OrderManager::RemoveOrder(uint64_t id) {
        auto it = m_orders.find(id);
        if (it == m_orders.end())
            throw std::runtime_error("Couldn't find Order with ID: " + std::to_string(id));

        auto order = it->second;
        auto obIt = m_orderBooks.find(order.SymbolId);
        obIt->second.RemoveOrder(order.Id);
        m_orders.erase(order.Id);
        return order;
    }

    void OrderManager::AddNonMatchOrder(Data::Order &order) {
        CORE_TRACE("Adding Order: {}", order);
        auto obIt = m_orderBooks.find(order.SymbolId);
        if (obIt == m_orderBooks.end()) {
            return;
        }

        auto &ob = obIt->second;

        // Order has not been fully filled, thus place it.
        m_orders[order.Id] = order;
        ob.AddOrder(m_orders.at(order.Id));
    }

    void OrderManager::AddOrder(Data::Order &order, std::vector<Data::OrderAction> &actions) {
        CORE_TRACE("Received Order: {}", order);
        auto obIt = m_orderBooks.find(order.SymbolId);
        if (obIt == m_orderBooks.end()) {
            return;
        }

        auto &ob = obIt->second;

        // Order has not been fully filled, thus place it.
        m_orders[order.Id] = order;
        ob.AddOrder(m_orders.at(order.Id));
    }

    void OrderManager::AddSymbol(const Data::Symbol &symbol) {
        // check if symbol already exists
        if (m_symbols.contains(symbol.Id))
            return;

        m_symbols[symbol.Id] = symbol;
        // create orderbook
        m_orderBooks.insert({symbol.Id, Data::OrderBook{symbol}});
    }

    Data::OrderBook* OrderManager::GetOrderBook(uint32_t symbolId) {
        auto res = m_orderBooks.find(symbolId);
        if (res == m_orderBooks.end()) {
            return nullptr;
        }

        return &(res->second);
    }

    std::vector<Data::Symbol> OrderManager::GetSymbols() {
        std::vector<Data::Symbol> symbols{m_symbols.size()};
        int i = 0;
        for (auto it = m_symbols.begin(); it != m_symbols.end(); ++it, ++i) {
            symbols.at(i) = it->second;
        }

        return symbols;
    }

    Data::Symbol const *OrderManager::GetSymbol(uint32_t id) {
        auto it = m_symbols.find(id);
        if (it == m_symbols.end())
            return nullptr;

        return &(it->second);
    }
}