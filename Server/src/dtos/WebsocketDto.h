//
// Created by danie on 11/29/2022.
//

#ifndef TRADINGENGINE_WEBSOCKETDTO_H
#define TRADINGENGINE_WEBSOCKETDTO_H

#include <nlohmann/json.hpp>

namespace TradingEngine::WsData {
    enum class OpCodes {
        LOGIN,
        SUBSCRIBE,
        UNSUBSCRIBE,
        READY,
        SUCCESS,
        ERROR
    };

    struct Payload {
        OpCodes opcode;
        nlohmann::json payload;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Payload, opcode, payload);

    struct Ack {
        OpCodes opcode;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Ack, opcode);

    enum class ErrorCodes {
        INVALID_FORMAT,
        INVALID_JSON,
        UNAUTHORIZED,
        NOT_FOUND
    };

    struct Error {
        ErrorCodes errorCode;
        std::string error;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Error, errorCode, error);

    struct Login {
        std::string apikey;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Login, apikey);

    struct SubSymbol {
        std::vector<uint32_t> symbolIds;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SubSymbol, symbolIds);
}

#endif //TRADINGENGINE_WEBSOCKETDTO_H
