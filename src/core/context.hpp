//
// Created by rokas on 3/2/26.
//

#ifndef DISK_ANALYSIS_CONTEXT_HPP
#define DISK_ANALYSIS_CONTEXT_HPP

#include <spdlog/logger.h>

#include "disk.hpp"
#include "logger.hpp"
#include "ui/cli/cli.hpp"

namespace da {
    class Ctx {
    public:
        static Ctx& get() {
            static Ctx ctx;
            return ctx;
        }

        CliConfig config;
        std::unique_ptr<Disk> disk;
        std::map<TSK_INUM_T, std::string> inodeDirMap{}; // inode -> path (to prevent infinite loops with hard links)

        Disk& getDisk() {
            if (!disk) {
                throw std::runtime_error("Disk not initialized");
            }
            return *disk;
        }

        spdlog::logger& getLogger() {
            if (!logger) {
                std::lock_guard<std::mutex> lock(_log_mutex);
                if (!logger) { // Check again after acquiring lock
                    logger = da::initLogger(config.log_file);
                }
            }
            return *logger;
        }

    private:
        Ctx() = default;
        Ctx(const Ctx&) = delete;
        Ctx& operator=(const Ctx&) = delete;

        std::mutex _log_mutex;
        std::shared_ptr<spdlog::logger> logger;
    };
};

#endif //DISK_ANALYSIS_CONTEXT_HPP