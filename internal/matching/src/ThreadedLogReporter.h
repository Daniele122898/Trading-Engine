//
// Created by danie on 9/22/2022.
//

#ifndef TRADINGENGINE_THREADEDLOGREPORTER_H
#define TRADINGENGINE_THREADEDLOGREPORTER_H

#include <OrderReport.h>
#include <log.h>
#include <thread>
#include <deque>

namespace TradingEngine::Matching {

    class ThreadedLogOrderReporter {
    public:
        ThreadedLogOrderReporter() {
            m_thread = std::thread([this] () {
                LoggingLoop();
            });
        }

        ~ThreadedLogOrderReporter() {
            m_running = false;
            // wait for thread to shut down
            CORE_INFO("Waiting for reporter thread to exit");
            if (m_thread.joinable())
                m_thread.join();
            CORE_INFO("Killed reporter thread");
        }

        void ReportOrderFill(OrderReport report) {
            std::lock_guard guard{m_mutex};
            m_reports.push_back(report);
        }

        ThreadedLogOrderReporter(const ThreadedLogOrderReporter& reporter) = delete;
        ThreadedLogOrderReporter operator=(const ThreadedLogOrderReporter& other) = delete;
        ThreadedLogOrderReporter(ThreadedLogOrderReporter&& reporter) = delete;
        ThreadedLogOrderReporter& operator=(ThreadedLogOrderReporter&& other) = delete;


    private:

        void LoggingLoop() {
            while (m_running) {
                m_mutex.lock();
                if (m_reports.empty())  {
                    m_mutex.unlock();
                    continue;
                }
                OrderReport report = m_reports.front();
                m_reports.pop_front();
                m_mutex.unlock();

                CORE_INFO("FILL REPORT: {} against {}: {} x {}",
                          report.OrderId,
                          report.AgainstId,
                          report.Price,
                          report.Quantity);
            }
        }


        std::deque<OrderReport> m_reports{};
        std::thread m_thread;
        std::mutex m_mutex{};
        bool m_running = true;
    };

} // Matching

#endif //TRADINGENGINE_THREADEDLOGREPORTER_H
