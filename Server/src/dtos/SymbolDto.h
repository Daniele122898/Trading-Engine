//
// Created by danie on 10/30/2022.
//

#ifndef TRADINGENGINE_SYMBOLDTO_H
#define TRADINGENGINE_SYMBOLDTO_H

#include <crow.h>
#include <nlohmann/json.hpp>
#include <symbol.h>

class SymbolDto : public crow::returnable {
public:
    explicit SymbolDto(TradingEngine::Data::Symbol const * symbol) :
        crow::returnable("application/json"), m_symbol{symbol} {};

    [[nodiscard]]
    std::string dump() const override {
        nlohmann::json j = {
                {"id", m_symbol->Id},
                {"ticker", m_symbol->Ticker}
        };

        return j.dump();
    }
private:
    TradingEngine::Data::Symbol const * m_symbol;
};

#endif //TRADINGENGINE_SYMBOLDTO_H
