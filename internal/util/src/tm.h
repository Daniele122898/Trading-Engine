//
// Created by danie on 12/9/2022.
//

#ifndef TRADINGENGINE_TIME_H
#define TRADINGENGINE_TIME_H

#include <chrono>
#include <ctime>
#include <utility>

namespace TradingEngine::Util {

    template<typename clock, typename duration>
    inline std::chrono::system_clock::time_point GetPointInToday(std::chrono::time_point<clock, duration> now, int hour, int minute, int second) {
        auto days = std::chrono::floor<std::chrono::days>(now);
        std::chrono::year_month_day ymd{days};

        std::tm timeinfo = std::tm();
        timeinfo.tm_year = static_cast<int>(ymd.year()) - 1900;
        timeinfo.tm_mon = static_cast<int>(static_cast<unsigned>(ymd.month())) - 1;
        timeinfo.tm_mday = static_cast<int>(static_cast<unsigned>(ymd.day()));

        timeinfo.tm_hour = hour;
        timeinfo.tm_min = minute;
        timeinfo.tm_sec = second;

        std::time_t tt = std::mktime(&timeinfo);
        return std::chrono::system_clock::from_time_t(tt);
    }

    inline std::chrono::system_clock::time_point GetPointInToday(int hour, int minute, int second) {
        auto now = std::chrono::system_clock::now();
        return GetPointInToday(now, hour, minute, second);
    }
}
#endif //TRADINGENGINE_TIME_H
