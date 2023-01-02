//
// Created by danie on 11/20/2022.
//

#ifndef STATSENGINE_DB_H
#define STATSENGINE_DB_H

#include <log.h>
#include <cstdint>
#include <string>
#include <pqxx/pqxx>
#include <symbol.h>
#include <order.h>

namespace StatsEngine {

    class Database {
    public:
        explicit Database(std::string connectionString) : m_conn{connectionString} {};

        std::vector<TradingEngine::Data::Symbol> GetSymbols() {
            pqxx::work txn{m_conn};
            pqxx::result r{txn.exec("SELECT id, ticker FROM public.symbols")};
            std::vector<TradingEngine::Data::Symbol> symbols{};
            symbols.reserve(r.size());

            for (auto row: r) {
                symbols.emplace_back(row[0].as<uint32_t>(), row[1].as<std::string>());
            }

            txn.commit();
            return symbols;
        }

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

#endif //STATSENGINE_DB_H
