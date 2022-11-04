//
// Created by danie on 11/4/2022.
//

#ifndef TRADINGENGINE_ORDERBOOKDTO_H
#define TRADINGENGINE_ORDERBOOKDTO_H

#include <crow.h>
#include <nlohmann/json.hpp>
#include <utility>
#include <order_book.h>
#include "SymbolDto.h"
#include "VectorReturnable.h"
#include "OrderDto.h"

class OrderBookDto : public crow::returnable {
public:
    explicit OrderBookDto(TradingEngine::Data::OrderBook const *ob) :
            crow::returnable("application/json"), m_orderBook{ob} {};

    [[nodiscard]]
    std::string dump() const override {
        auto j = toJson();
        return j.dump();
    }

    [[nodiscard]]
    nlohmann::json toJson() const {
        auto& bids = m_orderBook->Bids();
        std::vector<OrderDto> bidVec{};
        vectorizeLevels(bids, bidVec);

        auto& asks = m_orderBook->Asks();
        std::vector<OrderDto> askVec{};
        vectorizeLevels(asks, askVec);

        auto json = nlohmann::json{
                {"symbol", SymbolDto{m_orderBook->Symbol()}.toJson()},
                VectorReturnable{bidVec, "bids"}.toFlattenJson(),
                VectorReturnable{askVec, "asks"}.toFlattenJson()
        };

        return json;
    }

private:
    template<template<class> class S>
    void vectorizeLevels(std::set<TradingEngine::Data::Level, S<TradingEngine::Data::Level>> const &levels,
                        std::vector<OrderDto> &vec) const {

        for (auto& level : levels) {
            TradingEngine::Data::OrderNode *curr = level.Head;
            while (curr != nullptr) {
                vec.emplace_back(curr->Order);
                curr = curr->Next;
            }
        }

    }


private:
    TradingEngine::Data::OrderBook const *m_orderBook;
};


#endif //TRADINGENGINE_ORDERBOOKDTO_H
