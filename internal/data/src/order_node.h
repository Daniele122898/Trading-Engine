//
// Created by danie on 9/8/2022.
//

#ifndef TRADINGENGINE_ORDER_NODE_H
#define TRADINGENGINE_ORDER_NODE_H

#include "order.h"

namespace TradingEngine::Data {
    struct OrderNode {

        Order& mOrder;
        OrderNode *mPrev;
        OrderNode *mNext;

        explicit OrderNode(struct Order& order, struct
                OrderNode *prev = nullptr, struct OrderNode *next = nullptr) :
                mOrder{order}, mPrev{prev}, mNext{next} {}

        OrderNode(const OrderNode& node) = delete;
        OrderNode(OrderNode&& node) = delete;
        OrderNode operator=(const OrderNode& node) = delete;
        OrderNode operator=(OrderNode&& node) = delete;
    };
}

#endif //TRADINGENGINE_ORDER_NODE_H
