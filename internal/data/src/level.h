//
// Created by danie on 9/8/2022.
//

#ifndef TRADINGENGINE_LEVEL_H
#define TRADINGENGINE_LEVEL_H

#include <cstdint>
#include <unordered_map>
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
        OrderNode *Head = nullptr;
        OrderNode *Tail = nullptr;

        uint32_t Orders;
        uint64_t TotalVolume;

        Level() = delete;

        Level(int64_t price, OrderNode *head) :
                Price{price}, Orders{1}, TotalVolume{head->Order->CurrentQuantity} {
            AddOrder(head);
        }

        Level(int64_t price, Order *head);

        ~Level(){
            if (Head != nullptr) {
                delete Head;
                Head = nullptr;
            }
            if (Tail != nullptr) {
                delete Tail;
                Tail = nullptr;
            }
        }

        [[nodiscard]]
        inline LevelSide Side() const;
        [[nodiscard]]
        inline bool IsEmpty() const { return Head == nullptr; }

        inline void IncreaseVolume(uint64_t amount) { TotalVolume += amount; }
        inline void DecreaseVolume(uint64_t amount) { TotalVolume -= amount; }

        void AddOrder(OrderNode *order);
        void AddOrder(Order *order);
        void RemoveOrder(OrderNode *order);
        void RemoveOrder(Order *order);

        bool operator<(const Level& rhs) const {
            return Price < rhs.Price;
        }
        bool operator>(const Level& rhs) const {
            return Price > rhs.Price;
        }
        bool operator>=(const Level& rhs) const {
            return Price >= rhs.Price;
        }
        bool operator<=(const Level& rhs) const {
            return Price <= rhs.Price;
        }
        bool operator==(const Level& rhs) const {
            return Price == rhs.Price;
        }

        Level(const Level& node) = delete;
        Level(Level&& node) = delete;
        Level operator=(const Level& node) = delete;
        Level operator=(Level&& node) = delete;

    private:
        std::unordered_map<Order*,OrderNode*> m_orderMappings;
    };

} // Data

#endif //TRADINGENGINE_LEVEL_H
