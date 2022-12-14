//
// Created by danie on 9/22/2022.
//

#ifndef TRADINGENGINE_THREADEDLOGREPORTER_H
#define TRADINGENGINE_THREADEDLOGREPORTER_H

#include <OrderReport.h>
#include <log.h>
#include <thread>
#include <deque>
#include <readerwriterqueue.h>
#include <chrono>

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
            CORE_TRACE("Waiting for reporter thread to exit");
            if (m_thread.joinable())
                m_thread.join();
            CORE_TRACE("Killed reporter thread");
        }

        void ReportOrderFill(Data::Order const & order, Data::Order const & counterOrder, Data::FillReason reason, uint32_t diff = 0) {
            m_reports.emplace(order.Id, counterOrder.Id, order.Price, diff);
        }

        ThreadedLogOrderReporter(const ThreadedLogOrderReporter& reporter) = delete;
        ThreadedLogOrderReporter operator=(const ThreadedLogOrderReporter& other) = delete;
        ThreadedLogOrderReporter(ThreadedLogOrderReporter&& reporter) = delete;
        ThreadedLogOrderReporter& operator=(ThreadedLogOrderReporter&& other) = delete;

    private:

        void LoggingLoop() {
            while (m_running) {
                OrderReport report{};
                // block wait for new reports, timed to be killable
                if (!m_reports.wait_dequeue_timed(report, std::chrono::milliseconds(5)))
                    continue;

                CORE_INFO("FILL REPORT: {} against {}: {} x {}",
                          report.OrderId,
                          report.AgainstId,
                          report.Price,
                          report.Quantity);
            }
        }

        moodycamel::BlockingReaderWriterQueue<OrderReport> m_reports{1000};
        std::thread m_thread;
        bool m_running = true;
    };

} // Matching

#endif //TRADINGENGINE_THREADEDLOGREPORTER_H
