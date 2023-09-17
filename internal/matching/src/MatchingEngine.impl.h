//
// Created by ArgonautDev on 9/17/2023.
//

#pragma once

namespace TradingEngine::Matching {

    template<template<class> class S, typename Comp>
    void MatchingEngine::MatchFOK(Data::Order &order, std::set<Data::Level, S<Data::Level>> const &levels, Comp &compare,
                  std::vector<Data::OrderAction> &actions) {
        std::vector<Data::OrderNode *> orderNodes{};
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
        for (Data::OrderNode *node: orderNodes) {
            Data::Order &obOrder = node->Order;

            uint32_t diff = std::min(order.CurrentQuantity, obOrder.CurrentQuantity);
            order.CurrentQuantity -= diff;

            Data::Level *level = nullptr;
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
                m_orderManager.RemoveOrder(obOrder.Id);
            }
        }
    }

    template<typename S, typename Comp>
    bool MatchingEngine::MatchIOC(Data::Order &order, S &levels, Comp &compare, std::vector<Data::OrderAction> &actions) {
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
                    m_orderManager.RemoveOrder(obOrder.Id);
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
}