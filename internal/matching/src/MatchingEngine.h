//
// Created by danie on 9/10/2022.
//

#ifndef TRADINGENGINE_MATCHINGENGINE_H
#define TRADINGENGINE_MATCHINGENGINE_H

#include <cstdint>
#include <thread>
#include <vector>
#include <unordered_map>
#include <optional>
#include <list>

#include "order.h"
#include "readerwriterqueue.h"
#include "symbol.h"
#include "order_book.h"
#include "not_implemented_exception.h"
#include "log.h"
#include "MatchReporter.h"

namespace TradingEngine::Matching {

//    template<typename Logger> //, typename Persistence, typename Broadcaster
    class MatchingEngine {
    public:

//        explicit MatchingEngine(MatchReporter<Logger> reporter) :
//                m_reporter{std::move(reporter)} {
//        }

        explicit MatchingEngine() = default;

        std::optional<Data::Order> FindOrder(uint64_t id) {
            auto it = m_orders.find(id);
            if (it == m_orders.end()) {
                return std::nullopt;
            }

            return it->second;
        }

        Data::Order RemoveOrder(uint64_t id) {
            auto it = m_orders.find(id);
            if (it == m_orders.end())
                throw std::runtime_error("Couldn't find Order with ID: " + std::to_string(id));

            auto order = it->second;
            auto obIt = m_orderBooks.find(order.SymbolId);
            obIt->second.RemoveOrder(order.Id);
            m_orders.erase(order.Id);
            return order;
        }

        void AddNonMatchOrder(Data::Order& order) {
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

        void AddOrder(Data::Order &order, std::vector<Data::OrderAction>& actions) {
            CORE_TRACE("Received Order: {}", order);
            auto obIt = m_orderBooks.find(order.SymbolId);
            if (obIt == m_orderBooks.end()) {
                return;
            }

            auto &ob = obIt->second;

            // True if the match was complete -> Order either filled, or of kill variant
            bool finishedMatch = Match(order, ob, actions);
            ob.ClearEmptyLevels();
            if (finishedMatch) {
                CORE_TRACE("FINISHED MATCH, REPORT CANCELLATION");
                return;
            }

            // Order has not been fully filled, thus place it.
            m_orders[order.Id] = order;
            ob.AddOrder(m_orders.at(order.Id));
        }

        void AddSymbol(Data::Symbol const & symbol) {
            // check if symbol already exists
            if (m_symbols.contains(symbol.Id))
                return;

            m_symbols[symbol.Id] = symbol;
            // create orderbook
            m_orderBooks.insert({symbol.Id, Data::OrderBook{symbol}});
        }

        Data::OrderBook const *OrderBook(uint32_t id) {
            auto res = m_orderBooks.find(id);
            if (res == m_orderBooks.end()) {
                return nullptr;
            }

            return &(res->second);
        }

        std::vector<Data::Symbol> Symbols() {
            std::vector<Data::Symbol> symbols{m_symbols.size()};
            int i = 0;
            for (auto it = m_symbols.begin(); it != m_symbols.end(); ++it, ++i) {
                symbols.at(i) = it->second;
            }

            return symbols;
        }

        Data::Symbol const *Symbol(uint32_t id) {
            auto it = m_symbols.find(id);
            if (it == m_symbols.end())
                return nullptr;

            return &(it->second);
        }

    private:
        bool Match(Data::Order &order, Data::OrderBook &book, std::vector<Data::OrderAction>& actions) {
            switch (order.Type) {
                case Data::OrderType::MARKET:
                    if (!MatchMarket(order, book, actions)) {
                        actions.emplace_back(Data::Action::CANCELLED, order, std::nullopt, 0);
//                        m_reporter.ReportOrderFill(order, order, Data::Action::CANCELLED);
                    }
                    return true;
                case Data::OrderType::LIMIT:
                    return MatchIOC(order, book, actions);
                case Data::OrderType::FOK:
                    MatchFOK(order, book, actions);
                    return true;
                case Data::OrderType::IOC:
                    if (!MatchIOC(order, book, actions)) {
//                        m_reporter.ReportOrderFill(order, order, Data::Action::CANCELLED);
                        actions.emplace_back(Data::Action::CANCELLED, order, std::nullopt, 0);
                    }
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

        void MatchFOK(Data::Order &order, Data::OrderBook &book, std::vector<Data::OrderAction>& actions) {
            if (order.Side == Data::OrderSide::BUY) {
                MatchFOK(order, book.Asks(), m_greater, actions);
            } else {
                MatchFOK(order, book.Bids(), m_less, actions);
            }
        }

        template<template<class> class S, typename Comp>
        void MatchFOK(Data::Order &order, std::set<Data::Level, S<Data::Level>> const &levels, Comp &compare, std::vector<Data::OrderAction>& actions) {
            std::vector<Data::OrderNode* > orderNodes{};
            uint32_t currQ = order.CurrentQuantity;
            for (auto &lvl: levels) {
                if (currQ == 0)
                    break;

                auto &level = const_cast<Data::Level &>(lvl);
                if (compare(level.Price, order.Price)) {
                    CORE_TRACE("FOK limit {} {} next lvl price {}, aborting COLLECTION",
                               order.Price,
                               (order.Side == Data::OrderSide::BUY ? "below" : "above"),
                               level.Price);
                    break;
                }

                Data::OrderNode *curr = level.Head;
                while (curr != nullptr) {
                    Data::Order &obOrder = curr->Order;
                    // Check for self trade
                    if (obOrder.UserId == order.UserId) {
                        // TODO: maybe throw or error out
                        CORE_INFO("ENCOUNTERED SELF TRADE, ABORTING FILL {}\n{}", obOrder, order);
                        actions.emplace_back(Data::Action::SELF_TRADE, order, std::nullopt, 0);
//                        m_reporter.ReportOrderFill(order, order, Data::Action::SELF_TRADE);
                        return;
                    }

                    uint32_t diff = std::min(currQ, obOrder.CurrentQuantity);
                    currQ -= diff;
                    orderNodes.push_back(curr);

                    if (currQ == 0)
                        break;

                    curr = curr->Next;
                }
            }

            // Check if we can fill!
            if (currQ > 0) {
                // We can't fill, CANCEL!
                CORE_TRACE("Cant Fill FOK, CANCEL! Leftover: {}", currQ);
                actions.emplace_back(Data::Action::CANCELLED, order, std::nullopt, 0);
//                m_reporter.ReportOrderFill(order, order, Data::Action::CANCELLED);
                return;
            }

            // We can fill, thus match all orders
            for (Data::OrderNode* node : orderNodes) {
                Data::Order &obOrder = node->Order;

                uint32_t diff = std::min(order.CurrentQuantity, obOrder .CurrentQuantity);
                order.CurrentQuantity -= diff;

                Data::Level* level = nullptr;
                for (auto &lvl: levels) {
                    if (lvl.Price == obOrder.Price) {
                        level = &const_cast<Data::Level &>(lvl);
                        break;
                    }
                }
                if (level == nullptr) {
                    CORE_ERROR("TRYING TO FILL ORDER {} THAT DOESNT HAVE LEVEL WITH PRICE {}", obOrder, obOrder.Price);
                    return;
                }

                level->DecreaseVolume(diff);
                obOrder.CurrentQuantity -= diff;

                if (order.CurrentQuantity == 0) {
//                    m_reporter.ReportOrderFill(order, o, Data::Action::FILLED, diff);
                    actions.emplace_back(Data::Action::FILLED, order, obOrder, 0);
                }

                if (obOrder.CurrentQuantity == 0) {
//                    m_reporter.ReportOrderFill(o, order, Data::Action::FILLED, diff);
                    actions.emplace_back(Data::Action::FILLED, obOrder, order, 0);
//                    level->RemoveOrder(node);
                    RemoveOrder(obOrder.Id);
                }
            }
        }

        bool MatchMarket(Data::Order &order, Data::OrderBook &book, std::vector<Data::OrderAction>& actions) {
            // For now, we'll pretend a Market Order is basically an IOC with infinite price
            if (order.Side == Data::OrderSide::BUY) {
                order.Price = INT64_MAX;
                return MatchIOC(order, book.Asks(), m_greater, actions);
            } else {
                // let's not use a negative value here. We dont want the seller to have to pay
                // TODO: check correctness of this. Might be broken
                order.Price = 0;
                return MatchIOC(order, book.Bids(), m_less, actions);
            }
        }

        bool MatchIOC(Data::Order &order, Data::OrderBook &book, std::vector<Data::OrderAction>& actions) {
            if (order.Side == Data::OrderSide::BUY) {
                return MatchIOC(order, book.Asks(), m_greater, actions);
            } else {
                return MatchIOC(order, book.Bids(), m_less, actions);
            }
        }

        template<typename S, typename Comp>
        bool MatchIOC(Data::Order &order, S &levels, Comp &compare, std::vector<Data::OrderAction>& actions) {
            for (auto &lvl: levels) {
                auto &level = const_cast<Data::Level &>(lvl);
                if (compare(level.Price, order.Price)) {
                    CORE_TRACE("IOC limit {} {} next lvl price {}, aborting FILLING",
                            order.Price,
                            (order.Side == Data::OrderSide::BUY ? "below" : "above"),
                            level.Price);
                    break;
                }
                Data::OrderNode *obNode = level.Head;;
                while (obNode != nullptr) {
                    Data::Order &obOrder = obNode->Order;
                    // Check for self trade
                    if (obOrder.UserId == order.UserId) {
                        // TODO: maybe throw or error out
                        CORE_INFO("ENCOUNTERED SELF TRADE, ABORTING FILL {}\n{}", obOrder, order);
                        actions.emplace_back(Data::Action::SELF_TRADE, order, std::nullopt, 0);
//                        m_reporter.ReportOrderFill(order, order, Data::Action::SELF_TRADE);
                        return true;
                    }

                    uint32_t diff = std::min(order.CurrentQuantity, obOrder.CurrentQuantity);
                    order.CurrentQuantity -= diff;

                    Data::OrderNode *next = obNode->Next;

                    level.DecreaseVolume(diff);
                    obOrder.CurrentQuantity -= diff;

                    if (obOrder.CurrentQuantity == 0) {
                        actions.emplace_back(Data::Action::FILLED, obOrder, order, diff);
//                        m_reporter.ReportOrderFill(o, order, Data::Action::FILLED, diff);
//                        level.RemoveOrder(curr);
                        RemoveOrder(obOrder.Id);
                    }
                    obNode = next;

                    if (order.CurrentQuantity == 0) {
                        actions.emplace_back(Data::Action::FILLED, order, obOrder, diff);
//                        m_reporter.ReportOrderFill(order, o, Data::Action::FILLED, diff);
                        return true;
                    }
                }
            }

            if (order.CurrentQuantity > 0) {
                CORE_TRACE("Order with id {} could not be filled fully", order.Id);
                return false;
            }
            return true;
        }

        // MatchingEngine(const MatchingEngine &engine) = delete;
        // MatchingEngine(const MatchingEngine &&engine) = delete;
        // MatchingEngine operator=(const MatchingEngine &engine) = delete;
        // MatchingEngine operator=(const MatchingEngine &&engine) = delete;

        std::greater<int64_t> m_greater{};
        std::less<int64_t> m_less{};

        std::unordered_map<uint32_t, Data::Symbol> m_symbols{};

        std::unordered_map<uint32_t, Data::OrderBook> m_orderBooks{};
        std::unordered_map<uint64_t, Data::Order> m_orders{};

        // MatchReporter<Logger, Persistence, Broadcaster> m_reporter;
    };

} // Server

#endif //TRADINGENGINE_MATCHINGENGINE_H
