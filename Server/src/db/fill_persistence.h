//
// Created by danie on 11/22/2022.
//

#ifndef TRADINGENGINE_FILLPERSISTENCE_H
#define TRADINGENGINE_FILLPERSISTENCE_H

#include "db.h"
#include <cstdint>

namespace TradingEngine::Db {

    // TODO create threaded, non-blocking version of this
    class FillPersistence {
    public:
        explicit FillPersistence(Database& db): m_database{db} {}

        void UpdateOrderQuantity(Data::Order const & order, uint32_t newQuant) {
            m_database.UpdateOrder(order, newQuant);
        }

        void ReportOrderFill(Data::Order const & order, Data::Order const & counterOrder, Data::Action reason, uint32_t diff = 0) {
            m_database.AddFill(order, counterOrder, reason);
        }

    private:
        Database& m_database;
    };
}

#endif //TRADINGENGINE_FILLPERSISTENCE_H
