//
// Created by danie on 9/8/2022.
//

#ifndef TRADINGENGINE_LEVEL_H
#define TRADINGENGINE_LEVEL_H

#include <cstdint>
#include "order.h"
#include "order_node.h"

namespace TradingEngine::Data {

    enum class LevelSide {
        UNKNOWN,
        BID,
        ASK
    };

    struct Level {
        int64_t Price;
        OrderNode *Head;
        OrderNode *Tail;

        uint32_t Orders;
        uint64_t TotalVolume;

        Level() = default;

        Level(int64_t price, OrderNode *head) :
                Price{price}, Head{head}, Tail{head}, Orders{1}, TotalVolume{head->Order->CurrentQuantity} {}

        [[nodiscard]]
        inline LevelSide Side() const;

        [[nodiscard]]
        inline bool IsEmpty() const { return Head == nullptr; }

        inline void IncreaseVolume(uint64_t amount) { TotalVolume += amount; }

        inline void DecreaseVolume(uint64_t amount) { TotalVolume -= amount; }

        void AddOrder(OrderNode *order);

        void RemoveOrder(OrderNode *order);


    };

} // Data

#endif //TRADINGENGINE_LEVEL_H
