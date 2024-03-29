//
// Created by danie on 11/20/2022.
//

#ifndef STATSENGINE_STATSDB_H
#define STATSENGINE_STATSDB_H

#include <cstdint>
#include <pqxx/pqxx>
#include <string>

namespace StatsEngine::Db {

    class StatsDb {
    public:
        explicit StatsDb(std::string connStatsDb, std::string connEngineDb) : 
            m_statsConn{connStatsDb}, m_engineConn{connEngineDb} {};

        void CreateTablesIfNotExist();
        int64_t UpdateHistory(uint32_t symbolId, std::string_view startTime, std::string_view endTime, int64_t prevPrice);
        int64_t GetLastPrice(uint32_t symbolId);
        std::optional<std::tm> GetLastEndTime(uint32_t symboldId);
        std::optional<std::tm> GetFirstTimestamp(uint32_t symbolid);

    private:
        pqxx::connection m_statsConn;
        pqxx::connection m_engineConn;
    };
}

#endif //STATSENGINE_STATSDB_H
