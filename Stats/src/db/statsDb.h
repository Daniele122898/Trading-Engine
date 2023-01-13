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
        explicit StatsDb(std::string connectionString) : m_conn{connectionString} {};

        void CreateTablesIfNotExist();

    private:
        pqxx::connection m_conn;
    };
}

#endif //STATSENGINE_STATSDB_H
