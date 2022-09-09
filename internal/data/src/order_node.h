//
// Created by danie on 9/8/2022.
//

#ifndef TRADINGENGINE_ORDER_NODE_H
#define TRADINGENGINE_ORDER_NODE_H

#include "order.h"

namespace TradingEngine::Data {
    struct OrderNode {

        Order *Order;
        OrderNode *Prev;
        OrderNode *Next;

        OrderNode() = default;

        explicit OrderNode(struct Order *order) :
                Order{order}, Prev{nullptr}, Next{nullptr} {}

        OrderNode(struct Order *order, struct OrderNode *prev, struct OrderNode *next) :
                Order{order}, Prev{prev}, Next{next} {}
    };
}

#endif //TRADINGENGINE_ORDER_NODE_H
