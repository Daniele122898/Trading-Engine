//
// Created by danie on 9/10/2022.
//

#include "MatchingEngine.h"
#include "not_implemented_exception.h"
#include "log.h"

namespace TradingEngine::Matching {
    // TODO: Add proper errors
    void MatchingEngine::AddOrder(Data::Order &order) {
        CORE_TRACE("Received Order: {}", order);
        auto obIt = m_orderBooks.find(order.SymbolId);
        if (obIt == m_orderBooks.end()) {
            return;
        }

        auto ob = obIt->second;
        // First we add order, then we match

        // True if the match was complete -> Order either filled, or of kill variant
        if (Match(order, ob)) {
            return;
        }

        // Order has not been fully filled, thus place it.
        m_orders[order.Id] = order;
        ob.AddOrder(m_orders.at(order.Id));
    }

    // TODO: Add fill book to keep track of all the trades that actually happened lol
    bool MatchingEngine::Match(Data::Order &order, Data::OrderBook &book) {
        switch (order.Type) {
            case Data::OrderType::MARKET:
                MatchMarket(order, book);
                return true;
            case Data::OrderType::LIMIT:
                return MatchIOC(order, book);
            case Data::OrderType::FOK:
                throw Util::not_implemented_exception();
                break;
            case Data::OrderType::IOC:
                MatchIOC(order, book);
                return true;
            case Data::OrderType::STOP_MARKET:
                throw Util::not_implemented_exception();
                break;
            case Data::OrderType::STOP_LIMIT:
                throw Util::not_implemented_exception();
                break;
            case Data::OrderType::QUOTE:
                throw Util::not_implemented_exception();
                break;
        }
        return false;
    }

    bool MatchingEngine::MatchMarket(Data::Order &order, Data::OrderBook &book) {
        // For now, we'll pretend a Market Order is basically an IOC with infinite price
        if (order.Side == Data::OrderSide::BUY) {
            order.Price = INT64_MAX;
            return MatchIOC(order, book.Bids(), m_greater);
        } else {
            // let's not use a negative value here. We dont want the seller to have to pay
            // TODO: check correctness of this. Might be broken
            order.Price = 0;
            return MatchIOC(order, book.Asks(), m_less);
        }
    }

    bool MatchingEngine::MatchIOC(Data::Order &order, Data::OrderBook &book) {
        if (order.Side == Data::OrderSide::BUY) {
            return MatchIOC(order, book.Bids(), m_greater);
        } else {
            return MatchIOC(order, book.Asks(), m_less);
        }
    }

    template<typename S, typename Comp>
    bool MatchingEngine::MatchIOC(Data::Order &order, S &levels, Comp &compare) {
        for (auto &lvl: levels) {
            auto &level = const_cast<Data::Level &>(lvl);
            if (compare(level.Price, order.Price)) {
                CORE_TRACE("IOC limit {} {} next lvl price {}, aborting FILLING",
                           order.Price,
                           (order.Side == Data::OrderSide::BUY ? "above" : "below"),
                           level.Price);
                break;
            }
            Data::OrderNode *curr = level.Head;
            while (curr != nullptr) {
                Data::Order &o = curr->Order;
                uint32_t diff = std::min(order.CurrentQuantity, o.CurrentQuantity);
                order.CurrentQuantity -= diff;
                o.CurrentQuantity -= diff;

                Data::OrderNode *next = curr->Next;
                if (o.CurrentQuantity == 0) {
                    level.RemoveOrder(curr);
                } else {
                    level.DecreaseVolume(diff);
                }
                curr = next;

                if (order.CurrentQuantity > 0)
                    return true;
            }
        }

        if (order.CurrentQuantity > 0) {
            CORE_TRACE("Order with id {} could not be filled fully", order.Id);
            return false;
        }
        return true;
    }

    void MatchingEngine::AddSymbol(Data::Symbol const &symbol) {
        // check if symbol already exists
        if (m_symbols.contains(symbol.Id))
            return;

        m_symbols[symbol.Id] = symbol;
        // create orderbook
        m_orderBooks.insert({symbol.Id, Data::OrderBook{symbol}});
    }

    Data::OrderBook const *MatchingEngine::OrderBook(uint32_t id) {
        auto res = m_orderBooks.find(id);
        if (res == m_orderBooks.end()) {
            return nullptr;
        }

        return &(res->second);
    }

    std::vector<Data::Symbol> MatchingEngine::Symbols() {
        std::vector<Data::Symbol> symbols{m_symbols.size()};
        int i = 0;
        for (auto it = m_symbols.begin(); it != m_symbols.end(); ++it, ++i) {
            symbols.at(i) = it->second;
        }

        return symbols;
    }

    Data::Symbol const * MatchingEngine::Symbol(uint32_t id) {
        auto it = m_symbols.find(id);
        if (it == m_symbols.end())
            return nullptr;

        return &(it->second);
    }

} // Server