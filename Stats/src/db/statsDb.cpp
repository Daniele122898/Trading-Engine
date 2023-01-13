#include "statsDb.h"
#include <string>
#include <log.h>

namespace StatsEngine::Db {

        void StatsDb::CreateTablesIfNotExist() {
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
}
