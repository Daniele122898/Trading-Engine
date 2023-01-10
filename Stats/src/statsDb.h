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

                // txn.exec0(
                //         "CREATE TABLE IF NOT EXISTS public.users ( "
                //         "id bigserial PRIMARY KEY, "
                //         "username varchar(25) NOT NULL UNIQUE, "
                //         "email varchar(320) NOT NULL UNIQUE, "
                //         "password varchar(128) NOT NULL, "
                //         "salt varchar(128) NOT NULL, "
                //         "apikey varchar(128) NOT NULL UNIQUE "
                
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
