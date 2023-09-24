//
// Created by danie on 11/20/2022.
//

#ifndef TRADINGENGINE_DB_H
#define TRADINGENGINE_DB_H

#include <chrono>
#include <date/date.h>
#include <log.h>
#include <cstdint>
#include <sstream>
#include <string>
#include <pqxx/pqxx>
#include <symbol.h>
#include <order.h>

namespace TradingEngine::Db {

    class Database {
    public:
        explicit Database(std::string connectionString) : m_conn{connectionString} {};

        void CreateTablesIfNotExist() {
            try {
                pqxx::work txn{m_conn};

                txn.exec0(
                        "CREATE TABLE IF NOT EXISTS public.users ( "
                        "id bigserial PRIMARY KEY, "
                        "username varchar(25) NOT NULL UNIQUE, "
                        "email varchar(320) NOT NULL UNIQUE, "
                        "password varchar(128) NOT NULL, "
                        "salt varchar(128) NOT NULL, "
                        "apikey varchar(128) NOT NULL UNIQUE "
                        ")");

                txn.exec0(
                        "CREATE TABLE IF NOT EXISTS public.symbols ( "
                        "id serial PRIMARY KEY, "
                        "ticker varchar(6) NOT NULL UNIQUE "
                        ")");

//                txn.exec0(
//                        "CREATE TABLE IF NOT EXISTS public.orders ( "
//                        "id bigserial PRIMARY KEY, "
//                        "userId bigint REFERENCES users(id), "
//                        "symbolId integer REFERENCES symbols(id), "
//                        "type smallint NOT NULL, "
//                        "side smallint NOT NULL, "
//                        "lifetime smallint NOT NULL, "
//                        "price bigint NOT NULL, "
//                        "initialQ integer NOT NULL, "
//                        "currentQ integer NOT NULL, "
//                        "expiry date NOT NULL, "
//                        "creation timestamp NOT NULL "
//                        ")");

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
                        "diff integer NOT NULL, "
                        "expiry date NOT NULL, "
                        "creation timestamp NOT NULL, "
                        "filled_at timestamp NOT NULL, "
                        "counter_order_id bigint, "
                        "counter_user_id bigint REFERENCES users(id), "
                        "reason smallint NOT NULL "
                        ")");

//                txn.exec0(
//                        "CREATE OR REPLACE VIEW all_orders AS  "
//                        "SELECT * FROM ( "
//                        "select id, userId, symbolId, type, side, lifetime, price, "
//                        "initialQ, diff as currentQ,  "
//                        "expiry, creation, filled_at, counter_order_id, counter_user_id, reason, "
//                        "true isFill from fills union all  "
//                        "select id, userId, symbolId, type, side, lifetime, price, "
//                        "initialQ, currentQ,  "
//                        "expiry, creation, NULL as filled_at, NULL as counter_order_id, "
//                        "NULL as counter_user_id, NULL as reason, false isFill from orders) c"
//                );

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

//        std::vector<uint64_t> GetExpiredOrders() {
//            pqxx::work txn{m_conn};
//            pqxx::result r{txn.exec("SELECT id FROM public.orders WHERE "
//                                    "lifetime = 0 OR (lifetime = 1 AND expiry <= CURRENT_DATE)")};
//            std::vector<uint64_t> orderIds{};
//            orderIds.reserve(r.size());
//            for (auto row: r) {
//                orderIds.emplace_back(row[0].as<uint64_t>());
//            }
//
//            txn.commit();
//            return orderIds;
//        }

//        std::vector<Data::Order> GetOrders() {
//            pqxx::work txn{m_conn};
//            pqxx::result r{txn.exec(
//                    "SELECT id, userid, symbolid, type, side, "
//                    "lifetime, price, initialq, currentq, extract(epoch from expiry) as expiry, "
//                    "round(extract(epoch from creation)) as creation FROM public.orders")};
//            std::vector<Data::Order> orders{};
//            orders.reserve(r.size());
//
//            for (auto row: r) {
//                orders.emplace_back(row[0].as<uint64_t>(), row[1].as<uint64_t>(),
//                                    row[2].as<uint32_t>(),
//                                    static_cast<Data::OrderType>(row[3].as<int>()),
//                                    static_cast<Data::OrderSide>(row[4].as<int>()),
//                                    static_cast<Data::OrderLifetime>(row[5].as<int>()),
//                                    row[6].as<int64_t>(), row[7].as<uint32_t>(),
//                                    row[8].as<uint32_t>(), std::chrono::milliseconds(row[9].as<uint64_t>()),
//                                    std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>{std::chrono::seconds{row[10].as<uint64_t>()}});
//            }
//
//            txn.commit();
//            return orders;
//        }

        std::vector<TradingEngine::Data::Symbol> GetSymbols() {
            pqxx::work txn{m_conn};
            pqxx::result r{txn.exec("SELECT id, ticker FROM public.symbols")};
            std::vector<Data::Symbol> symbols{};
            symbols.reserve(r.size());

            for (auto row: r) {
                symbols.emplace_back(row[0].as<uint32_t>(), row[1].as<std::string>());
            }

            txn.commit();
            return symbols;
        }

        uint32_t AddSymbol(const std::string &ticker) {
            pqxx::work txn{m_conn};
            txn.exec0("INSERT INTO public.symbols(ticker) "
                      "VALUES ('" + ticker + "')");

            uint32_t currSymbolId = txn.query_value<uint32_t>(
                    "SELECT last_value FROM symbols_id_seq"
            );
            txn.commit();

            return currSymbolId;
        }
        
        uint64_t LargestFillId() {
            pqxx::work txn{m_conn};
            pqxx::result r{txn.exec("SELECT id from public.fills ORDER BY id DESC LIMIT 1")};
            if (r.empty())
                return 0;

            txn.commit();
            return r[0][0].as<uint64_t>();
        }

        uint64_t LastUsedOrderId() {
            pqxx::work txn{m_conn};
            uint64_t lastOrderId = txn.query_value<uint64_t>(
                    "SELECT last_value FROM orders_id_seq"
            );
            txn.commit();
            return lastOrderId;
        }
        
        void ProcessActions(Data::Order& originOrder, std::vector<Data::OrderAction>& actions) {
            pqxx::work txn{m_conn};
            auto stream = pqxx::stream_to::table(
                txn,
                "public.fills",
                std::vector<std::string>{"id", "userid", "symbolid", "type", "side", "lifetime", "price",
                    "initialq", "diff", "expiry", "creation", "filled_at", "counter_order_id", "counter_user_id",
                    "reason"}
            );

            for(auto& action : actions)
            {
                if (action.reason != Data::Action::FILLED)
                    continue;
                if (!action.counterOrder.has_value()) {
                    CORE_WARN("Counter order not set in action even though its a fill!");
                    continue;
                }

                auto tp = std::chrono::system_clock::time_point(action.order.ExpiryMs);
                auto days = std::chrono::floor<std::chrono::days>(tp);
                std::chrono::year_month_day ymd{days};
                std::stringstream date;
                date << static_cast<int>(ymd.year()) << "/" 
                    << static_cast<int>(static_cast<unsigned>(ymd.month()))
                    << "/" << static_cast<int>(static_cast<unsigned>(ymd.day()));

                stream.write_values(action.order.Id, action.order.UserId, action.order.SymbolId,
                        static_cast<int>(action.order.Type), 
                        static_cast<int>(action.order.Side),
                        static_cast<int>(action.order.Lifetime),
                        action.order.Price,
                        action.order.InitialQuantity,
                        action.quantity,
                        date.str(),
                        date::format( "%F %T%z", action.order.CreationTp),
                        date::format( "%F %T%z", std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now())),
                        action.counterOrder->Id,
                        action.counterOrder->UserId,
                        static_cast<int>(action.reason)
                    );
            }
            stream.complete();
            txn.commit();
        }

//        uint64_t AddOrder(Data::Order const &order) {
//            pqxx::work txn{m_conn};
//            std::stringstream query;
//            query << "INSERT INTO public.orders(userid, symbolid, type, side, "
//                  << "lifetime, price, initialq, currentq, expiry, creation) "
//                  << "VALUES ("
//                  << order.UserId << ", "
//                  << order.SymbolId << ", "
//                  << static_cast<int>(order.Type) << ", "
//                  << static_cast<int>(order.Side) << ", "
//                  << static_cast<int>(order.Lifetime) << ", "
//                  << order.Price << ", "
//                  << order.InitialQuantity << ", "
//                  << order.CurrentQuantity << ", "
//                  << "to_timestamp(" << (order.ExpiryMs.count() / 1000.0) << ")::date, "
//                  << "CURRENT_TIMESTAMP "
//                  << ")";
//            txn.exec0(query.str());
//
//            uint64_t currOrderId = txn.query_value<uint64_t>(
//                    "SELECT last_value FROM orders_id_seq"
//            );
//            txn.commit();
//
//            return currOrderId;
//        }

//        void UpdateOrder(Data::Order const & order, uint32_t newQuant) {
//            pqxx::work txn{m_conn};
//
//            // if this fails, we'll fail the entire transaction
//            txn.exec0("UPDATE public.orders SET currentq=" + std::to_string(newQuant) + " WHERE id=" + std::to_string(order.Id));
//            txn.commit();
//
//            return;
//        }

//        void AddFill(Data::Order const &order, Data::Order const &counterOrder, Data::Action reason) {
//            pqxx::work txn{m_conn};
//
//
//            std::stringstream query;
//            query << "INSERT INTO public.fills(id, userid, symbolid, type, side, "
//                  << "lifetime, price, initialq, diff, expiry, creation, "
//                     "filled_at, counter_order_id, counter_user_id, reason) "
//                  << "VALUES ("
//                  << order.Id << ", "
//                  << order.UserId << ", "
//                  << order.SymbolId << ", "
//                  << static_cast<int>(order.Type) << ", "
//                  << static_cast<int>(order.Side) << ", "
//                  << static_cast<int>(order.Lifetime) << ", "
//                  << order.Price << ", "
//                  << order.InitialQuantity << ", "
//                  << order.CurrentQuantity << ", "
//                  << "to_timestamp(" << (order.ExpiryMs.count() / 1000.0) << ")::date, "
//                  // << "to_timestamp(" << (order.CreationTp.time_since_epoch().count() / 1000.0) << "), "
//                  // TODO: Fix this inefficiency
//                  << "(SELECT creation from public.orders WHERE id = " + std::to_string(order.Id) + "),"
//                  << "CURRENT_TIMESTAMP, ";
//
//            if (reason == Data::Action::FILLED) {
//                query << counterOrder.Id << ", "
//                      << counterOrder.UserId << ", ";
//            } else {
//                query << "NULL , NULL , ";
//            }
//            query << static_cast<int>(reason) << ")";
//            txn.exec0(query.str());
//
//            // if this fails, we'll fail the entire transaction
//            txn.exec0("DELETE FROM public.orders WHERE Id = " + std::to_string(order.Id));
//
//            txn.commit();
//        }

        bool TryGetUserId(std::string &apikey, uint64_t &id) {
            pqxx::work txn{m_conn};
            pqxx::result r{txn.exec("SELECT id FROM public.users WHERE apikey = " + m_conn.quote(apikey))};
            if (r.empty())
                return false;

            id = r[0][0].as<uint64_t>();
            txn.commit();

            return true;
        }

        bool TryGetUser(std::string &username, uint64_t &id, std::basic_string<std::byte> &password,
                        std::basic_string<std::byte> &salt, std::string &apikey) {
            pqxx::work txn{m_conn};
            pqxx::result r{txn.exec(
                    "SELECT id, password, salt, apikey FROM public.users WHERE username = " + m_conn.quote(username))};
            if (r.empty())
                return false;

            id = r[0][0].as<uint64_t>();
            password = m_conn.unesc_bin(r[0][1].as<std::string>());
            salt = m_conn.unesc_bin(r[0][2].as<std::string>());
            apikey = r[0][3].as<std::string>();

            txn.commit();

            return true;
        }

        uint64_t AddUser(std::string username, std::string email, unsigned char *pwhash, unsigned char *salt,
                         unsigned char *apikey) {
            pqxx::work txn{m_conn};

            std::string password(reinterpret_cast<char *>(pwhash), 32);
            std::string saltstr(reinterpret_cast<char *>(salt), 32);
            std::string apikeystr(reinterpret_cast<char *>(apikey), 32);

            std::stringstream query;
            query << "INSERT INTO public.users(username, email, password, salt, apikey) VALUES ("
                  << m_conn.quote(username) << ", "
                  << m_conn.quote(email) << ", "
                  << m_conn.quote(pqxx::binary_cast(password)) << ", "
                  << m_conn.quote(pqxx::binary_cast(saltstr)) << ", "
                  << "encode(" << m_conn.quote(pqxx::binary_cast(apikeystr)) << "::bytea, 'base64')"
                  << ")";

            txn.exec0(query.str());

            uint64_t currUserId = txn.query_value<uint64_t>(
                    "SELECT last_value FROM users_id_seq"
            );

            txn.commit();

            return currUserId;
        }

    private:
        pqxx::connection m_conn;
    };
}

#endif //TRADINGENGINE_DB_H
