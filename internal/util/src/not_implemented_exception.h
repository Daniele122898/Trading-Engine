//
// Created by danie on 9/11/2022.
//

#ifndef TRADINGENGINE_NOT_IMPLEMENTED_EXCEPTION_H
#define TRADINGENGINE_NOT_IMPLEMENTED_EXCEPTION_H

#include <stdexcept>

namespace TradingEngine::Util {
    class not_implemented_exception : public std::logic_error {
    public:
        not_implemented_exception() :
                std::logic_error("Function not yet implemented") {};
    };
}

#endif //TRADINGENGINE_NOT_IMPLEMENTED_EXCEPTION_H
