#pragma once

#include <nlohmann/json.hpp>
#include "OrderDto.h"
#include "VectorReturnable.h"
#include "crow/returnable.h"
#include "order.h"

namespace TradingEngine
{
    class OrderActionDto : public crow::returnable {
    public:
        OrderActionDto(Data::OrderAction& orderAction) :
            crow::returnable("application/json"), m_orderAction{orderAction} {};

        std::string dump() const override {
            auto j = toJson();
            return j.dump();
        }

        nlohmann::json toJson() const {
            auto json = nlohmann::json {
                {"reaosn", m_orderAction.reason},
                {"order", OrderDto{m_orderAction.order}.toJson()},
                {"quantity", m_orderAction.quantity}
            };

            if (m_orderAction.counterOrder)
                json["counterOrder"] = OrderDto(m_orderAction.counterOrder.value()).toJson();

            return json;
        }

        static std::vector<OrderActionDto> ToVector(std::vector<Data::OrderAction>& actions) {
            std::vector<OrderActionDto> dtos{};
            for (auto& action: actions) {
                dtos.push_back(OrderActionDto{action});
            }

            return dtos;
        }

    private:
        Data::OrderAction& m_orderAction;
        
    };
}
