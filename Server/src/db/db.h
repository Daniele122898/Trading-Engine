//
// Created by danie on 11/20/2022.
//

#ifndef TRADINGENGINE_DB_H
#define TRADINGENGINE_DB_H

#include <log.h>
#include <cstdint>
#include <string>
#include <pqxx/pqxx>
#include <symbol.h>
#include <order.h>

namespace TradingEngine::Db {

    class Database {
    public:
        explicit Database(std::string connectionString): m_conn{connectionString} {};

        void CreateTablesIfNotExist() {
            try {
                pqxx::work txn{m_conn};

                txn.exec0(
                        "CREATE TABLE IF NOT EXISTS public.users ( "
                        "id bigserial PRIMARY KEY, "
                        "username varchar(25) NOT NULL UNIQUE, "
                        "password varchar(256) NOT NULL, "
                        "salt varchar(256) NOT NULL, "
                        "apikey varchar(256) NOT NULL UNIQUE "
                        ")");

                txn.exec0(
                        "CREATE TABLE IF NOT EXISTS public.symbols ( "
                        "id serial PRIMARY KEY, "
                        "ticker varchar(6) NOT NULL UNIQUE "
                        ")");

                txn.exec0(
                        "CREATE TABLE IF NOT EXISTS public.orders ( "
                        "id bigserial PRIMARY KEY, "
                        "userId bigint REFERENCES users(id), "
                        "symbolId integer REFERENCES symbols(id), "
                        "type smallint NOT NULL, "
                        "side smallint NOT NULL, "
                        "lifetime smallint NOT NULL, "
                        "price bigint NOT NULL, "
                        "initialQ integer NOT NULL, "
                        "currentQ integer NOT NULL, "
                        "expiry date NOT NULL, "
                        "creation timestamp NOT NULL "
                        ")");

                txn.exec0(
                        "CREATE TABLE IF NOT EXISTS public.fills ( "
                        "id bigint PRIMARY KEY, "
                        "userId bigint REFERENCES users(id), "
                        "symbolId integer REFERENCES symbols(id), "
                        "type smallint NOT NULL, "
                        "side smallint NOT NULL, "
                        "lifetime smallint NOT NULL, "
                        "price bigint NOT NULL, "
                        "initialQ integer NOT NULL, "
                        "finalQ integer NOT NULL, "
                        "expiry date NOT NULL, "
                        "creation timestamp NOT NULL, "
                        "filled_at timestamp NOT NULL, "
                        "counter_order_id bigint, "
                        "counter_user_id bigint REFERENCES users(id), "
                        "reason smallint NOT NULL "
                        ")");

                txn.exec0(
                        "CREATE OR REPLACE VIEW all_orders AS  "
                        "SELECT * FROM ( "
                        "select id, userId, symbolId, type, side, lifetime, price, "
                        "initialQ, finalQ as currentQ,  "
                        "expiry, creation, filled_at, counter_order_id, counter_user_id, reason, "
                        "true isFill from fills union all  "
                        "select id, userId, symbolId, type, side, lifetime, price, "
                        "initialQ, currentQ,  "
                        "expiry, creation, NULL as filled_at, NULL as counter_order_id, "
                        "NULL as counter_user_id, NULL as reason, false isFill from orders) c"
                );

                txn.commit();

            }
            catch (pqxx::sql_error const &e) {
                CORE_ERROR("SQL ERROR: {}", e.what());
                CORE_ERROR("QUERY: {}", e.query());
            }
            catch (std::exception const &e) {
                CORE_ERROR("ERROR: {}", e.what());
            }
        }

        std::vector<TradingEngine::Data::Symbol> GetSymbols() {
            pqxx::work txn{m_conn};
            pqxx::result r{txn.exec("SELECT id, ticker FROM public.symbols")};
            std::vector<Data::Symbol> symbols{};
            symbols.reserve(r.size());

            for (auto row : r) {
                symbols.emplace_back(row[0].as<uint32_t>(), row[1].as<std::string>());
            }

            txn.commit();
            return symbols;
        }

        uint32_t AddSymbol(const std::string& ticker) {
            pqxx::work txn{m_conn};
            txn.exec0("INSERT INTO public.symbols(ticker) "
                      "VALUES ('" + ticker + "')");

            uint32_t currSymbolId = txn.query_value<uint32_t>(
                    "SELECT last_value FROM symbols_id_seq"
            );
            txn.commit();

            return currSymbolId;
        }

        uint64_t LastUsedOrderId() {
            pqxx::work txn{m_conn};
            uint64_t lastOrderId = txn.query_value<uint64_t>(
                    "SELECT last_value FROM orders_id_seq"
            );
            txn.commit();
            return lastOrderId;
        }

        uint64_t AddOrder(Data::Order const & order) {
            pqxx::work txn{m_conn};
            std::stringstream query;
            query << "INSERT INTO public.orders(userid, symbolid, type, side, "
                << "lifetime, price, initialq, currentq, expiry, creation) "
                << "VALUES ("
                << order.UserId << ", "
                << order.SymbolId << ", "
                << static_cast<int>(order.Type) << ", "
                << static_cast<int>(order.Side) << ", "
                << static_cast<int>(order.Lifetime) << ", "
                << order.Price << ", "
                << order.InitialQuantity << ", "
                << order.CurrentQuantity << ", "
                << "to_timestamp(" << (order.ExpiryMs.count() / 1000.0) << ")::date, "
                << "to_timestamp(" << (order.CreationTp.time_since_epoch().count() / 1000.0) << ") "
                << ")";
            txn.exec0(query.str());

            uint64_t currOrderId = txn.query_value<uint64_t>(
                    "SELECT last_value FROM orders_id_seq"
            );
            txn.commit();

            return currOrderId;
        }

        void AddFill(Data::Order const & order, Data::Order const & counterOrder, Data::FillReason reason) {
            pqxx::work txn{m_conn};
            std::stringstream query;
            query << "INSERT INTO public.fills(id, userid, symbolid, type, side, "
                  << "lifetime, price, initialq, finalq, expiry, creation, "
                     "filled_at, counter_order_id, counter_user_id, reason) "
                  << "VALUES ("
                  << order.Id << ", "
                  << order.UserId << ", "
                  << order.SymbolId << ", "
                  << static_cast<int>(order.Type) << ", "
                  << static_cast<int>(order.Side) << ", "
                  << static_cast<int>(order.Lifetime) << ", "
                  << order.Price << ", "
                  << order.InitialQuantity << ", "
                  << order.CurrentQuantity << ", "
                  << "to_timestamp(" << (order.ExpiryMs.count() / 1000.0) << ")::date, "
                  << "to_timestamp(" << (order.CreationTp.time_since_epoch().count() / 1000.0) << ") "
                  << "CURRENT_TIMESTAMP, ";

            if (reason == Data::FillReason::FILLED) {
                query << counterOrder.Id << ", "
                      << counterOrder.UserId << ", ";
            } else {
                query << "NULL , NULL , ";
            }
            query << static_cast<int>(reason) << ")";

            txn.exec0(query.str());
            txn.commit();
        }

    private:
        pqxx::connection m_conn;
    };
}

#endif //TRADINGENGINE_DB_H
