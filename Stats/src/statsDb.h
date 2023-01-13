//
// Created by danie on 11/20/2022.
//

#ifndef STATSENGINE_STATSDB_H
#define STATSENGINE_STATSDB_H

#include <log.h>
#include <cstdint>
#include <string>
#include <pqxx/pqxx>
#include <symbol.h>
#include <order.h>

namespace StatsEngine {

    class StatsDb {
    public:
        explicit StatsDb(std::string connectionString) : m_conn{connectionString} {};

        void CreateTablesIfNotExist() {
            try {
                pqxx::work txn{m_conn};

                txn.exec0(
                        "CREATE TABLE IF NOT EXISTS public.price_history ("
                        "id bigserial PRIMARY KEY,"
                        "symbol_id integer NOT NULL,"
                        "start_time timestamp NOT NULL,"
                        "end_time timestamp NOT NULL,"
                        "price bigint NOT NULL,"
                        "volume bigint NOT NULL"
                        ")"
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

    private:
        pqxx::connection m_conn;
    };
}

#endif //STATSENGINE_STATSDB_H
