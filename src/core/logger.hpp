//
// Created by Rokas Morkūnas on 03/03/2026.
//

#ifndef DISK_ANALYSIS_LOGGER_HPP
#define DISK_ANALYSIS_LOGGER_HPP

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace da {
    inline std::shared_ptr<spdlog::logger> initLogger(const std::string& log_file, const bool& log_console = false) {
        try {
            std::vector<spdlog::sink_ptr> sinks;

            if (log_console) {
                auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                console_sink->set_level(spdlog::level::debug);
                sinks.push_back(console_sink);
            }

            if (!log_file.empty()) {
                auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file, true);
                file_sink->set_level(spdlog::level::debug);
                sinks.push_back(file_sink);
            }

            auto logger = std::make_shared<spdlog::logger>("da_logger", sinks.begin(), sinks.end());
            logger->set_level(spdlog::level::debug);

            spdlog::register_logger(logger);
            return logger;
        } catch (const spdlog::spdlog_ex& ex) {
            throw std::runtime_error(std::string("Logger initialization failed: ") + ex.what());
        }
    }
} // da

#endif //DISK_ANALYSIS_LOGGER_HPP