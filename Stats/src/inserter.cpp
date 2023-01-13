#include "inserter.h"
#include <chrono>

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
                 
            }
        }
    }
}
