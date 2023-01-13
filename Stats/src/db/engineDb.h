//
// Created by danie on 11/20/2022.
//

#ifndef STATSENGINE_ENGINEDB_H
#define STATSENGINE_ENGINEDB_H

#include <cstdint>
#include <string>
#include <pqxx/pqxx>
#include <symbol.h>

namespace StatsEngine::Db {

    class EngineDb {
    public:
        explicit EngineDb(std::string connectionString) : m_conn{connectionString} {};

        std::vector<TradingEngine::Data::Symbol> GetSymbols();
        bool TryGetUserId(std::string &apikey, uint64_t &id);
        bool TryGetUser(std::string &username, uint64_t &id, std::basic_string<std::byte> &password,
                        std::basic_string<std::byte> &salt, std::string &apikey);
        uint64_t AddUser(std::string username, std::string email, unsigned char *pwhash, unsigned char *salt,
                         unsigned char *apikey);

    private:
        pqxx::connection m_conn;
    };
}

#endif //STATSENGINE_ENGINEDB_H
