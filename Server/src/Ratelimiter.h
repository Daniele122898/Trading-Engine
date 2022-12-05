//
// Created by danie on 12/5/2022.
//

#ifndef TRADINGENGINE_RATELIMITER_H
#define TRADINGENGINE_RATELIMITER_H

#include <unordered_map>
#include <vector>
#include <cstdint>
#include <chrono>

namespace TradingEngine {

    // TODO: maybe we need smth for get vs post
    enum class BUCKET_TYPE {
        SIMPLE,
        ORDER_BOOK,
        LISTS,
    };

    class Ratelimiter {
    public:
        Ratelimiter() {
            m_maxRequests[static_cast<uint8_t>(BUCKET_TYPE::SIMPLE)] = 50;
            m_maxRequests[static_cast<uint8_t>(BUCKET_TYPE::ORDER_BOOK)] = 10;
            m_maxRequests[static_cast<uint8_t>(BUCKET_TYPE::LISTS)] = 25;

            m_requestCounts[static_cast<uint8_t>(BUCKET_TYPE::SIMPLE)] = {};
            m_requestCounts[static_cast<uint8_t>(BUCKET_TYPE::ORDER_BOOK)] = {};
            m_requestCounts[static_cast<uint8_t>(BUCKET_TYPE::LISTS)] = {};

            m_resetTimes[static_cast<uint8_t>(BUCKET_TYPE::SIMPLE)] = {};
            m_resetTimes[static_cast<uint8_t>(BUCKET_TYPE::ORDER_BOOK)] = {};
            m_resetTimes[static_cast<uint8_t>(BUCKET_TYPE::LISTS)] = {};
        }

        bool IsRatelimited(BUCKET_TYPE type, uint64_t userId);

    private:
        std::vector<uint8_t> m_maxRequests{};
        std::chrono::seconds m_interval = std::chrono::seconds(5);

        std::vector<std::unordered_map<uint64_t, uint8_t>> m_requestCounts{};
        std::vector<std::unordered_map<uint64_t, std::chrono::system_clock::time_point>> m_resetTimes{};
    };

} // TradingEngine


#endif //TRADINGENGINE_RATELIMITER_H
