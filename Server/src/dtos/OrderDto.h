//
// Created by danie on 11/4/2022.
//

#ifndef TRADINGENGINE_ORDERDTO_H
#define TRADINGENGINE_ORDERDTO_H

#include <crow.h>
#include <nlohmann/json.hpp>
#include <utility>
#include <order.h>

class OrderDto : public crow::returnable {
public:
    explicit OrderDto(TradingEngine::Data::Order& order) :
            crow::returnable("application/json"), m_order{order} {};

    [[nodiscard]]
    std::string dump() const override {
        auto j = toJson();
        return j.dump();
    }

    [[nodiscard]]
    nlohmann::json toJson() const {
        return nlohmann::json {
                {"id", m_order.Id},
                {"userId", m_order.UserId},
                {"symbolId", m_order.SymbolId},
                {"type", m_order.Type},
                {"side", m_order.Side},
                {"lifetime", m_order.Lifetime},
                {"price", m_order.Price},
                {"initialQ", m_order.InitialQuantity},
                {"currentQ", m_order.CurrentQuantity},
        };
    }

private:
    TradingEngine::Data::Order& m_order;
};

#endif //TRADINGENGINE_ORDERDTO_H
