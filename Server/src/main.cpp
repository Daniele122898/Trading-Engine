#include <log.h>
#include <MatchingEngine.h>
#include <crow.h>
#include "ThreadedLogReporter.h"

#include "dtos/SymbolDto.h"
#include "dtos/VectorReturnable.h"
#include "dtos/OrderBookDto.h"

#include "db/db.h"

#include <chrono>
#include <iostream>

using namespace TradingEngine;

namespace TradingEngine::Data {
    void from_json(nlohmann::json const &json, Order &order) {
        // TODO: Add validation
        json.at("id").get_to(order.Id);
        json.at("userId").get_to(order.UserId);
        json.at("symbolId").get_to(order.SymbolId);
        json.at("type").get_to(order.Type);
        json.at("side").get_to(order.Side);
        json.at("lifetime").get_to(order.Lifetime);
        json.at("price").get_to(order.Price);
        json.at("initialQ").get_to(order.InitialQuantity);
        json.at("initialQ").get_to(order.CurrentQuantity);
    }
}

int main() {

//    std::chrono::time_point currently = std::chrono::time_point_cast<std::chrono::milliseconds>(
//            std::chrono::system_clock::now()
//    );
//    std::chrono::duration millis_since_utc_epoch = currently.time_since_epoch();

    Util::log::Init("Matching Engine");

    TradingEngine::Db::Database db{"postgres://postgres:test123@localhost:5432/trading_test"};
    db.CreateTablesIfNotExist();

    uint64_t lastOrderId = db.LastUsedOrderId();
    CORE_TRACE("Last used order ID: {}", lastOrderId);

    Matching::MatchingEngine<Matching::ThreadedLogOrderReporter> engine{
            Matching::MatchReporter(std::make_unique<Matching::ThreadedLogOrderReporter>())};


    auto symbols = db.GetSymbols();
    for (auto& symb : symbols) {
        CORE_TRACE("Adding symbol {} {}", symb.Id, symb.Ticker);
        engine.AddSymbol(symb);
    }

    Data::Order order(++lastOrderId,
                      1,
                      1,
                      TradingEngine::Data::OrderType::LIMIT,
                      TradingEngine::Data::OrderSide::BUY,
                      TradingEngine::Data::OrderLifetime::GFD,
                      10,
                      10);

    Data::Order order2(++lastOrderId,
                       1,
                       1,
                       TradingEngine::Data::OrderType::LIMIT,
                       TradingEngine::Data::OrderSide::BUY,
                       TradingEngine::Data::OrderLifetime::GFD,
                       9,
                       10);

    engine.AddOrder(order);
    engine.AddOrder(order2);

    crow::SimpleApp app;

#ifndef NDEBUG
    app.loglevel(crow::LogLevel::Debug);
#else
    app.loglevel(crow::LogLevel::Warning);
#endif

    // Endpoints
    CROW_ROUTE(app, "/symbol").methods("POST"_method)([&engine, &db](const crow::request &req) {
        if (req.body.empty()) {
            return crow::response{400};
        }

        // TODO check parsing
        auto json = nlohmann::json::parse(req.body);

        // TODO ERROR HANDLING
        auto id = db.AddSymbol(json["ticker"]);
        CORE_TRACE("Adding symbol {} with id {}", json["ticker"], id);

        Data::Symbol symbol{id, json["ticker"]};
        engine.AddSymbol(symbol);

        return crow::response{crow::status::OK};
    });

    CROW_ROUTE(app, "/symbols").methods("GET"_method)([&engine]() {
        auto symbols = engine.Symbols();
        std::vector<SymbolDto> symbolsDto{};
        symbolsDto.reserve(symbols.size());

        for (auto &s: symbols) {
            SymbolDto sdto{std::move(s)};
            symbolsDto.push_back(sdto);
        }

        return VectorReturnable{std::move(symbolsDto), "symbols"};

    });

    CROW_ROUTE(app, "/symbol/<uint>").methods("GET"_method)([&engine](unsigned int symbolId) {
        auto symbol = engine.Symbol(symbolId);
        if (symbol == nullptr) {
            return crow::response{404};
        }
        return crow::response{SymbolDto{*symbol}};
    });

    CROW_ROUTE(app, "/orderbook/<uint>").methods("GET"_method)([&engine](unsigned int orderBookId) {
        Data::OrderBook const *book = engine.OrderBook(orderBookId);
        if (book == nullptr) {
            return crow::response{404};
        }
        return crow::response{OrderBookDto{book}};
    });

    CROW_ROUTE(app, "/order").methods("POST"_method)([&engine, &lastOrderId](const crow::request &req) {
        if (req.body.empty()) {
            return crow::response{400};
        }

        // TODO check parsing
        auto json = nlohmann::json::parse(req.body);
        json["id"] = ++lastOrderId;
        auto order = json.get<Data::Order>();
        engine.AddOrder(order);

        return crow::response{crow::status::OK};
    });

    CORE_TRACE("Starting Server on port 18080");
    app.port(18080).multithreaded().run();
}

