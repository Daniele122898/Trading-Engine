//
// Created by danie on 9/11/2022.
//

#include "log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace TradingEngine::Util {
    spdlog::logger* log::s_CoreLogger;

    void log::Init(const std::string& assemblyName, spdlog::level::level_enum level, spdlog::level::level_enum flushLevel) {
        auto colorSink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
        colorSink->set_pattern("%^[%T] [%n] :: %v%$");
        s_CoreLogger = new spdlog::logger(assemblyName, colorSink);

        s_CoreLogger->set_level(level);
        s_CoreLogger->flush_on(flushLevel);
    }
} // Util