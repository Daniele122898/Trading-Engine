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
        SELF_TRADE,
        CANCELLED,
        FILLED,
        EXPIRED,
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

    struct ShareCounter {
        uint64_t orderId;
        uint64_t counterId;
        uint32_t diff;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ShareCounter, orderId, counterId, diff);

    struct Share {
        uint64_t orderId;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Share, orderId);

    struct ShareReport{
        ShareReport() {}

        ShareReport(uint64_t orderId, uint64_t counterId, uint32_t symbolId, uint32_t diff, OpCodes opCode)
        : OrderId{orderId}, CounterId{counterId}, SymbolId{symbolId}, Diff{diff}, OpCode{opCode} {}

        uint64_t OrderId;
        uint64_t CounterId;
        uint32_t SymbolId;
        uint32_t Diff;
        OpCodes OpCode;
    };
}

#endif //TRADINGENGINE_WEBSOCKETDTO_H
