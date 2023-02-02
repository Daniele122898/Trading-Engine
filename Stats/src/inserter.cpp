#include "inserter.h"
#include "log.h"
#include <chrono>
#include <cstdint>

namespace StatsEngine {

    void Inserter::HandlerLoop() {
        while (m_running) {
            auto now = std::chrono::system_clock::now();

            if (now <= tp) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            // TODO: Fix time skew (we always want 5 minute intervals, but this is slowly going to skew) 
            tp += std::chrono::minutes(5);

            for (auto& symb : m_symbols) {
                CORE_TRACE("Updating Price history for {} : {}", symb.Id, symb.Ticker);
                auto tmo = m_statsDb.GetLastEndTime(symb.Id);
                if (!tmo.has_value()) {
                    tmo = m_statsDb.GetFirstTimestamp(symb.Id);
                    if (!tmo.has_value())
                        continue;
                    NormalizeTimeStamp(tmo.value());
                }
                auto tm = tmo.value();
                auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm) - timezone);
                int64_t prev_price = m_statsDb.GetLastPrice(symb.Id);
                while (tp < now) {
                    // get Starttime
                    std::ostringstream st{};
                    st << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

                    // get endtime
                    std::ostringstream et{};
                    auto etm = GetNextInterval(tm);
                    et << std::put_time(&etm, "%Y-%m-%d %H:%M:%S");
                    CORE_TRACE("Calculating pricepoint for time interval: {} - {}", st.str(), et.str());
                    prev_price = m_statsDb.UpdateHistory(symb.Id, st.str(), et.str(), prev_price);

                    // move interval
                    tm = etm;
                    tp += std::chrono::minutes(5); 
                } 
            }
        }
    }

    std::tm Inserter::GetNextInterval(std::tm&  start) {
        // TODO: Check the rigorousness of this timezone approach
        auto tp = std::chrono::system_clock::from_time_t(std::mktime(&start) - timezone); 
        tp += std::chrono::minutes(5);
        auto ett = std::chrono::system_clock::to_time_t(tp);
        std::tm etm = *std::gmtime(&ett);
        return etm;
    }

    void Inserter::NormalizeTimeStamp(std::tm& tm) {
        if (tm.tm_min % 10 >= 5) {
            tm.tm_min -= (tm.tm_min % 10) - 5;
            tm.tm_sec = 0;
        } else {
            tm.tm_min -= tm.tm_min % 10;
            tm.tm_sec = 0;
        }
    }
}
