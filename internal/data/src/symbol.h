//
// Created by danie on 9/7/2022.
//

#ifndef TRADINGENGINE_SYMBOL_H
#define TRADINGENGINE_SYMBOL_H

#include <cstdint>
#include <cstring>

namespace TradingEngine::Data {
    struct Symbol {
        uint32_t Id;
        char Ticker[8]{};

        Symbol() = default;
        Symbol(uint32_t id, const char name[8]): Id{id} {
            // We want the symbol to own the memory
            std::memcpy(Ticker, name, sizeof(Ticker));
        }
        ~Symbol() = default;

        Symbol(const Symbol&) = default;
        Symbol(Symbol&& sym) = default;

        Symbol& operator=(const Symbol&) noexcept = default;
        Symbol& operator=(Symbol&&) noexcept = default;
    };
}

#endif //TRADINGENGINE_SYMBOL_H
