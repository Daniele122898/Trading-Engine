//
// Created by danie on 9/8/2022.
//

#include "level.h"

namespace TradingEngine::Data {
    LevelSide Level::Side() const {
        if (Head == nullptr) {
            return LevelSide::UNKNOWN;
        }

        return Head->Order->Side == OrderSide::BUY ? LevelSide::BID : LevelSide::ASK;
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
        TotalVolume += order->Order->CurrentQuantity;
    }

    void Level::RemoveOrder(OrderNode *order) {
        order->Prev->Next = order->Next;
        order->Next->Prev = order->Prev;

        --Orders;
        TotalVolume -= order->Order->CurrentQuantity;

        delete order;
    }
} // Data