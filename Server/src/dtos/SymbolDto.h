//
// Created by danie on 10/30/2022.
//

#ifndef TRADINGENGINE_SYMBOLDTO_H
#define TRADINGENGINE_SYMBOLDTO_H

#include <crow.h>
#include <nlohmann/json.hpp>
#include <utility>
#include <symbol.h>

class SymbolDto : public crow::returnable {
public:
    explicit SymbolDto(TradingEngine::Data::Symbol symbol) :
        crow::returnable("application/json"), m_symbol{std::move(symbol)} {};

    [[nodiscard]]
    std::string dump() const override {
        auto j = toJson();
        return j.dump();
    }

    [[nodiscard]]
    nlohmann::json toJson() const {
        return nlohmann::json {
                {"id", m_symbol.Id},
                {"ticker", m_symbol.Ticker}
        };
    }

private:
    TradingEngine::Data::Symbol m_symbol;
};

#endif //TRADINGENGINE_SYMBOLDTO_H
