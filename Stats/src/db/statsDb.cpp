#include "statsDb.h"
#include <cmath>
#include <cstdint>
#include <string>
#include <log.h>
#include <sys/types.h>

namespace StatsEngine::Db {

        void StatsDb::CreateTablesIfNotExist() {
            try {
                pqxx::work txn{m_statsConn};

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

        // TODO: Fix pricing, potentially do volume adjusted average price
        void StatsDb::UpdateHistory(uint32_t symbolId, std::string& startTime, std::string& endTime, int64_t prevPrice) {
            pqxx::work txn{m_engineConn};
            
            std::stringstream buySideQuery;
            buySideQuery << "SELECT avg(price) as price, count(id) as volume FROM public.fills "
                << "WHERE symbolid = " << std::to_string(symbolId) << " "
                << "AND filled_at >= '" << startTime << "' "
                << "AND filled_at < '" << endTime << "' "
                << "AND side = 0";

            std::stringstream sellSideQuery;
            sellSideQuery << "SELECT avg(price) as price, count(id) as volume FROM public.fills "
                << "WHERE symbolid = " << std::to_string(symbolId) << " "
                << "AND filled_at >= '" << startTime << "' "
                << "AND filled_at < '" << endTime << "' "
                << "AND side = 1";

            pqxx::result buySide{txn.exec(buySideQuery.str())};
            pqxx::result sellSide{txn.exec(sellSideQuery.str())};
            int64_t price = 0;
            uint64_t volume = 0;
            if (buySide.empty() && sellSide.empty()) {
               price = prevPrice; 
            } else if (buySide.empty()) {
                price = std::round(sellSide[0][0].as<double>());
                volume = sellSide[0][1].as<uint64_t>();
            } else if (sellSide.empty()) {
                price = std::round(buySide[0][0].as<double>());
                volume = buySide[0][1].as<uint64_t>();
            } else {
                int64_t buyPrice = std::round(buySide[0][0].as<double>());
                int64_t sellPrice = std::round(sellSide[0][0].as<double>());
                price = std::round((buyPrice + sellPrice) / 2.0); 
                volume = buySide[0][1].as<uint64_t>() + sellSide[0][1].as<uint64_t>();
            }
            txn.commit();

            pqxx::work statsTxn{m_statsConn};
            std::stringstream insertQuery;
            insertQuery << "INSERT INTO public.price_history(symbol_id, start_time, end_time, price, volume) "
                  << "VALUES ("
                  << symbolId << ", "
                  << "'" << startTime << "', "
                  << "'" << endTime << "', "
                  << price << ", "
                  << volume << ")";

            statsTxn.exec0(insertQuery.str());
            statsTxn.commit();
        }
    
}
