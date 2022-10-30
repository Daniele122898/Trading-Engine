//
// Created by danie on 9/7/2022.
//

#ifndef TRADINGENGINE_SYMBOL_H
#define TRADINGENGINE_SYMBOL_H

#include <cstdint>
#include <utility>
#include <string>

namespace TradingEngine::Data {
    struct Symbol {
        uint32_t Id;
        std::string Ticker;

        Symbol() = default;
        Symbol(uint32_t id, std::string  ticker): Id{id}, Ticker{std::move(ticker)} {}
        ~Symbol() = default;

        Symbol(const Symbol&) = default;
        Symbol(Symbol&& sym) = default;

        Symbol& operator=(const Symbol&) noexcept = default;
        Symbol& operator=(Symbol&&) noexcept = default;
    };
}

#endif //TRADINGENGINE_SYMBOLDTO_H
