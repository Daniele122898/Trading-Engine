//
// Created by danie on 12/5/2022.
//

#include "Ratelimiter.h"

namespace TradingEngine {
    // TODO think about the locking
    bool Ratelimiter::IsRatelimited(BUCKET_TYPE type, uint64_t userId) {
        // Check if the rate limit has been exceeded for this user
        auto& rc = m_requestCounts[static_cast<uint8_t>(type)];
        auto& rt = m_resetTimes[static_cast<uint8_t>(type)];
        auto maxReq = m_maxRequests[static_cast<uint8_t>(type)];
        auto it = rc.find(userId);
        if (it == rc.end()) {
            rc[userId] = 1;
            rt[userId] = std::chrono::system_clock::now();
            return false;
        }
        if (it->second > maxReq) return true;

        // Increment the request count for this user
        ++rc[userId];

        // Calculate the time until the rate limit is reset for this user
        auto now = std::chrono::system_clock::now();
        auto resetTime = rt[userId] + std::chrono::seconds(m_interval);
        if (now >= resetTime) {
            rt[userId] = now;
            rc[userId] = 0;
        }

        // Return true if the request is allowed
        return false;
    }
} // TradingEngine