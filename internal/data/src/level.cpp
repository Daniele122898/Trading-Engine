//
// Created by danie on 9/8/2022.
//

#include "level.h"

namespace TradingEngine::Data {
    Level::Level(int64_t price, Order &head)  :
            Price{price}, Orders{1}, TotalVolume{head.CurrentQuantity} {

        auto orderNode = new OrderNode(head);
        AddOrder(orderNode);
    }

    LevelSide Level::Side() const {
        if (Head == nullptr) {
            return LevelSide::UNKNOWN;
        }

        return Head->mOrder.Side == OrderSide::BUY ? LevelSide::BID : LevelSide::ASK;
    }

    void Level::AddOrder(OrderNode *order) {
        if (Head == nullptr) {
            Head = order;
            Tail = order;
        } else {
            Tail->mNext = order;
            order->mPrev = Tail;
            Tail = order;
        }

        ++Orders;
        TotalVolume += order->mOrder.CurrentQuantity;

        m_orderMappings[order->mOrder.Id] = order;
    }

    void Level::RemoveOrder(OrderNode *order) {
        if (order == Head)
            Head = order->mNext;
        if (order == Tail)
            Tail = order->mPrev;
        if (order->mPrev != nullptr)
            order->mPrev->mNext = order->mNext;
        if (order->mNext != nullptr)
            order->mNext->mPrev = order->mPrev;

        --Orders;
        TotalVolume -= order->mOrder.CurrentQuantity;

        m_orderMappings.erase(order->mOrder.Id);
        delete order;
    }

    void Level::AddOrder(Order &order) {
        auto node = new OrderNode(order);
        AddOrder(node);
    }

    void Level::RemoveOrder(Order &order) {
        auto orderNode = m_orderMappings[order.Id];
        RemoveOrder(orderNode);
    }
} // Data