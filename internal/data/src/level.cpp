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

        return Head->Order.Side == OrderSide::BUY ? LevelSide::BID : LevelSide::ASK;
    }

    void Level::AddOrder(OrderNode *order) {
        if (Head == nullptr) {
            Head = order;
            Tail = order;
        } else {
            Tail->Next = order;
            order->Prev = Tail;
            Tail = order;
        }

        ++Orders;
        TotalVolume += order->Order.CurrentQuantity;

        m_orderMappings[order->Order.Id] = order;
    }

    void Level::RemoveOrder(OrderNode *order) {
        if (order == Head)
            Head = order->Next;
        if (order == Tail)
            Tail = order->Prev;
        if (order->Prev != nullptr)
            order->Prev->Next = order->Next;
        if (order->Next != nullptr)
            order->Next->Prev = order->Prev;

        --Orders;
        TotalVolume -= order->Order.CurrentQuantity;

        m_orderMappings.erase(order->Order.Id);
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