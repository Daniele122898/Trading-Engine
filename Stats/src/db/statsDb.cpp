#include "statsDb.h"
#include <chrono>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <log.h>
#include <string_view>
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
        
        std::optional<std::tm> StatsDb::GetLastEndTime(uint32_t symbolId) {
            pqxx::work txn{m_statsConn};

            std::string query = "SELECT date_trunc('second', end_time) "
	                            "FROM public.price_history "
                                "WHERE symbol_id = " + std::to_string(symbolId) + 
                                " ORDER BY end_time DESC LIMIT 1";

            pqxx::result res{txn.exec(query)};
            if (res.empty()) {
                return {};
            }

            std::istringstream is{res[0][0].as<std::string>()};
            std::tm tm = {};
            is >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
            return tm;
        }

        std::optional<std::tm> StatsDb::GetFirstTimestamp(uint32_t symbolid) {
            pqxx::work txn{m_engineConn};

            std::string query = "SELECT date_trunc('second', filled_at) "
                    "FROM public.fills as f "
                    "WHERE symbolid = " + std::to_string(symbolid) + " AND reason = 2 "
                    "ORDER BY filled_at ASC LIMIT 1";

            pqxx::result res{txn.exec(query)};
            if (res.empty()) {
                return {};
            }
            std::istringstream is{res[0][0].as<std::string>()};
            std::tm tm = {};
            is >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
            return tm;
        }
        
        int64_t StatsDb::GetLastPrice(uint32_t symbolId) {
            pqxx::work txn{m_statsConn};

            std::string query = "SELECT price "
	                            "FROM public.price_history "
                                "WHERE symbol_id = " + std::to_string(symbolId) +  
                                " ORDER BY end_time DESC LIMIT 1";

            pqxx::result res{txn.exec(query)};
            if (res.empty()) {
                return 0;
            }

            int64_t price = res[0][0].as<int64_t>();
            txn.commit();

            return price;
        }

        // TODO: Fix pricing, potentially do volume adjusted average price
        int64_t StatsDb::UpdateHistory(uint32_t symbolId, std::string_view startTime, std::string_view endTime, int64_t prevPrice) {
            pqxx::work txn{m_engineConn};
            
            std::stringstream buySideQuery;

            buySideQuery << "SELECT avg(CASE WHEN f.type != 0 THEN f.price "
                << "ELSE (SELECT price FROM public.all_orders AS ao WHERE ao.id = f.counter_order_id) END) "
                << "as price, count(id) as volume FROM public.fills as f "
                << "WHERE symbolid = " << std::to_string(symbolId) << " "
                << "AND filled_at >= '" << startTime << "' "
                << "AND filled_at < '" << endTime << "' "
                << "AND side = 0";

            std::stringstream sellSideQuery;
            sellSideQuery << "SELECT avg(CASE WHEN f.type != 0 THEN f.price "
                << "ELSE (SELECT price FROM public.all_orders AS ao WHERE ao.id = f.counter_order_id) END) " 
                << "as price, count(id) as volume FROM public.fills as f "
                << "WHERE symbolid = " << std::to_string(symbolId) << " "
                << "AND filled_at >= '" << startTime << "' "
                << "AND filled_at < '" << endTime << "' "
                << "AND side = 1";

            pqxx::result buySide{txn.exec(buySideQuery.str())};
            pqxx::result sellSide{txn.exec(sellSideQuery.str())};
            int64_t price = 0;
            uint64_t sellVol = sellSide[0][1].as<uint64_t>();
            uint64_t buyVol = buySide[0][1].as<uint64_t>();
            uint64_t volume = 0;
            if (buyVol == 0 && sellVol == 0) {
               price = prevPrice; 
            } else if (buyVol == 0) {
                price = std::round(sellSide[0][0].as<double>());
                volume = sellVol;
            } else if (sellVol == 0) {
                price = std::round(buySide[0][0].as<double>());
                volume = buyVol;
            } else {
                int64_t buyPrice = std::round(buySide[0][0].as<double>());
                int64_t sellPrice = std::round(sellSide[0][0].as<double>());
                price = std::round((buyPrice + sellPrice) / 2.0); 
                volume = buyVol + sellVol;
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

            return price;
        }
    
}
