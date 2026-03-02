//
// Created by rokas on 3/2/26.
//

#ifndef DISK_ANALYSIS_CONTEXT_HPP
#define DISK_ANALYSIS_CONTEXT_HPP

#include "disk.hpp"
#include "../ui/cli/cli.hpp"

namespace da {
    class Ctx {
    public:
        static Ctx& get() {
            static Ctx ctx;
            return ctx;
        }

        CliConfig config;
        std::unique_ptr<Disk> disk;

        Disk& getDisk() {
            if (!disk) {
                throw std::runtime_error("Disk not initialized");
            }
            return *disk;
        }

    private:
        Ctx() = default;
        Ctx(const Ctx&) = delete;
        Ctx& operator=(const Ctx&) = delete;
    };
};

#endif //DISK_ANALYSIS_CONTEXT_HPP