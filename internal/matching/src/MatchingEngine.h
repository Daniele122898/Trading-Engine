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

#include "order.h"
#include "readerwriterqueue.h"
#include "symbol.h"
#include "order_book.h"
#include "not_implemented_exception.h"
#include "log.h"
#include "MatchReporter.h"

namespace TradingEngine::Matching {

    template<typename Logger, typename Persistence, typename Broadcaster>
    class MatchingEngine {
    public:

        explicit MatchingEngine(MatchReporter<Logger, Persistence, Broadcaster> reporter) :
                m_reporter{std::move(reporter)} {

            m_thread = std::thread([this]() {
                    MatchingLoop();
            });
        }

        ~MatchingEngine() {
            m_running = false;
            CORE_TRACE("Waiting for Matching Engine Thread to exit");
            if (m_thread.joinable())
                m_thread.join();
            CORE_TRACE("Killed Matching Engine Thread");
        }

        std::optional<Data::Order> FindOrder(uint64_t id) {
            auto it = m_orders.find(id); 
            if (it == m_orders.end()) {
                return std::nullopt;
            }

            return it->second;
        }
        
        void CreateReport(Data::Order order, Data::FillReason reason) {
            m_reporter.ReportOrderFill(order, order, reason);
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
        
        void AddOrderQueue(Data::Order &order) {
            m_orderQueue.enqueue(order);    
        }

        void AddOrder(Data::Order &order) {
            CORE_TRACE("Received Order: {}", order);
            auto obIt = m_orderBooks.find(order.SymbolId);
            if (obIt == m_orderBooks.end()) {
                return;
            }

            auto &ob = obIt->second;

            // True if the match was complete -> Order either filled, or of kill variant
            bool finishedMatch = Match(order, ob);
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

        void MatchingLoop() {
            while (m_running) {
                Data::Order order;
                if (!m_orderQueue.try_dequeue(order)) 
                    continue;

                AddOrder(order);
            }

        }

        bool Match(Data::Order &order, Data::OrderBook &book) {
            switch (order.Type) {
                case Data::OrderType::MARKET: 
                    if (!MatchMarket(order, book)) {
                        m_reporter.ReportOrderFill(order, order, Data::FillReason::CANCELLED);
                    }
                    return true;
                case Data::OrderType::LIMIT:
                    return MatchIOC(order, book);
                case Data::OrderType::FOK:
                    MatchFOK(order, book);
                    return true;
                case Data::OrderType::IOC:
                    if (!MatchIOC(order, book)) {
                        m_reporter.ReportOrderFill(order, order, Data::FillReason::CANCELLED);
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

        void MatchFOK(Data::Order &order, Data::OrderBook &book) {
            if (order.Side == Data::OrderSide::BUY) {
                MatchFOK(order, book.Asks(), m_greater);
            } else {
                MatchFOK(order, book.Bids(), m_less);
            }
        }

        template<template<class> class S, typename Comp>
        void MatchFOK(Data::Order &order, std::set<Data::Level, S<Data::Level>> const &levels, Comp &compare) {
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
                    Data::Order &o = curr->Order;
                    // Check for self trade
                    if (o.UserId == order.UserId) {
                        // TODO: maybe throw or error out
                        CORE_INFO("ENCOUNTERED SELF TRADE, ABORTING FILL {}\n{}", o, order);
                        m_reporter.ReportOrderFill(order, order, Data::FillReason::SELF_TRADE);
                        return;
                    }

                    uint32_t diff = std::min(currQ, o.CurrentQuantity);
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
                m_reporter.ReportOrderFill(order, order, Data::FillReason::CANCELLED);
                return;
            }

            // We can fill, thus match all orders
            for (Data::OrderNode* node : orderNodes) {
                Data::Order &o = node->Order;

                uint32_t diff = std::min(order.CurrentQuantity, o.CurrentQuantity);
                order.CurrentQuantity -= diff;

                Data::Level* level = nullptr;
                for (auto &lvl: levels) {
                    if (lvl.Price == o.Price) {
                        level = &const_cast<Data::Level &>(lvl);
                        break;
                    }
                }
                if (level == nullptr) {
                    CORE_ERROR("TRYING TO FILL ORDER {} THAT DOESNT HAVE LEVEL WITH PRICE {}", o, o.Price);
                    return;
                }

                level->DecreaseVolume(diff);
                o.CurrentQuantity -= diff;

                if (order.CurrentQuantity == 0) {
                    m_reporter.ReportOrderFill(order, o, Data::FillReason::FILLED, diff);
                }

                if (o.CurrentQuantity == 0) {
                    m_reporter.ReportOrderFill(o, order, Data::FillReason::FILLED, diff);
//                    level->RemoveOrder(node);
                    RemoveOrder(o.Id);
                } else {
                    m_reporter.UpdateOrder(o, o.CurrentQuantity);
                }

            }
        }

        bool MatchMarket(Data::Order &order, Data::OrderBook &book) {
            // For now, we'll pretend a Market Order is basically an IOC with infinite price
            if (order.Side == Data::OrderSide::BUY) {
                order.Price = INT64_MAX;
                return MatchIOC(order, book.Asks(), m_greater);
            } else {
                // let's not use a negative value here. We dont want the seller to have to pay
                // TODO: check correctness of this. Might be broken
                order.Price = 0;
                return MatchIOC(order, book.Bids(), m_less);
            }
        }

        bool MatchIOC(Data::Order &order, Data::OrderBook &book) {
            if (order.Side == Data::OrderSide::BUY) {
                return MatchIOC(order, book.Asks(), m_greater);
            } else {
                return MatchIOC(order, book.Bids(), m_less);
            }
        }

        template<typename S, typename Comp>
        bool MatchIOC(Data::Order &order, S &levels, Comp &compare) {
            for (auto &lvl: levels) {
                auto &level = const_cast<Data::Level &>(lvl);
                if (compare(level.Price, order.Price)) {
                    CORE_TRACE("IOC limit {} {} next lvl price {}, aborting FILLING",
                               order.Price,
                               (order.Side == Data::OrderSide::BUY ? "below" : "above"),
                               level.Price);
                    break;
                }
                Data::OrderNode *curr = level.Head;
                while (curr != nullptr) {
                    Data::Order &o = curr->Order;
                    // Check for self trade
                    if (o.UserId == order.UserId) {
                        // TODO: maybe throw or error out
                        CORE_INFO("ENCOUNTERED SELF TRADE, ABORTING FILL {}\n{}", o, order);
                        m_reporter.ReportOrderFill(order, order, Data::FillReason::SELF_TRADE);
                        return true;
                    }

                    uint32_t diff = std::min(order.CurrentQuantity, o.CurrentQuantity);
                    order.CurrentQuantity -= diff;

                    Data::OrderNode *next = curr->Next;

                    level.DecreaseVolume(diff);
                    o.CurrentQuantity -= diff;

                    if (o.CurrentQuantity == 0) {
                        m_reporter.ReportOrderFill(o, order, Data::FillReason::FILLED, diff);
//                        level.RemoveOrder(curr);
                        RemoveOrder(o.Id);
                    } else {
                        m_reporter.UpdateOrder(o, o.CurrentQuantity);
                    }
                    curr = next;

                    if (order.CurrentQuantity == 0) {
                        m_reporter.ReportOrderFill(order, o, Data::FillReason::FILLED, diff);
                        return true;
                    }
                }
            }

            if (order.CurrentQuantity > 0) {
                CORE_TRACE("Order with id {} could not be filled fully", order.Id);
                m_reporter.UpdateOrder(order, order.CurrentQuantity);
                return false;
            }
            return true;
        }

        MatchingEngine(const MatchingEngine &engine) = delete;
        MatchingEngine(const MatchingEngine &&engine) = delete;
        MatchingEngine operator=(const MatchingEngine &engine) = delete;
        MatchingEngine operator=(const MatchingEngine &&engine) = delete;

        std::greater<int64_t> m_greater{};
        std::less<int64_t> m_less{};

        std::unordered_map<uint32_t, Data::Symbol> m_symbols{};

        std::unordered_map<uint32_t, Data::OrderBook> m_orderBooks{};
        std::unordered_map<uint64_t, Data::Order> m_orders{};

        MatchReporter<Logger, Persistence, Broadcaster> m_reporter;

        // Run on separate thread to ensure single threaded nature of matching engine 
        moodycamel::BlockingReaderWriterQueue<Data::Order> m_orderQueue{200};
        std::thread m_thread;
        bool m_running = true;

    };

} // Server

#endif //TRADINGENGINE_MATCHINGENGINE_H
