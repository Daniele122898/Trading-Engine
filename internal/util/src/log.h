//
// Created by danie on 9/11/2022.
//

#ifndef TRADINGENGINE_LOG_H
#define TRADINGENGINE_LOG_H

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace TradingEngine::Util {

    class log {
    public:
        static void Init(const std::string& assemblyName, spdlog::level::level_enum level, spdlog::level::level_enum flushLevel);

        static spdlog::logger* GetLogger() { return s_CoreLogger; }

    private:
        static spdlog::logger* s_CoreLogger;
    };

} // Util

// Core log macros
#define CORE_TRACE(...)    ::TradingEngine::Util::log::GetLogger()->trace(__VA_ARGS__)
#define CORE_INFO(...)     ::TradingEngine::Util::log::GetLogger()->info(__VA_ARGS__)
#define CORE_WARN(...)     ::TradingEngine::Util::log::GetLogger()->warn(__VA_ARGS__)
#define CORE_ERROR(...)    ::TradingEngine::Util::log::GetLogger()->error(__VA_ARGS__)
#define CORE_CRITICAL(...) ::TradingEngine::Util::log::GetLogger()->critical(__VA_ARGS__)



#endif //TRADINGENGINE_LOG_H
