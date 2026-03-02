//
// Created by rokas on 3/2/26.
//

#ifndef DISK_ANALYSIS_WRAPPERS_HPP
#define DISK_ANALYSIS_WRAPPERS_HPP

#include "actions/partitions.hpp"
#include "core/context.hpp"

namespace da::cli {
    inline void listPartitions() {
        auto& ctx = Ctx::get();
        auto partitions = Partitions::listPartitions(ctx.getDisk());

        if (ctx.config.json_output) {
            nlohmann::json j = partitions;
            std::cout << j.dump() << std::endl;
        } else if (ctx.config.csv_output) {
            std::cout << "id,name,byte_offset,byte_length,has_filesystem,fs_type\n";
            for (const auto& p : partitions) {
                std::cout << p.id << ","
                          << "\"" << p.name << "\","
                          << p.byte_offset << ","
                          << p.byte_length << ","
                          << (p.has_filesystem ? "true" : "false") << ","
                          << "\"" << p.fs_type << "\"\n";
            }
        } else {
            std::cout << "ID\tName\tByte Offset\tByte Length\tHas Filesystem\tFilesystem Type\n";
            for (const auto& p : partitions) {
                std::cout << p.id << "\t"
                          << p.name << "\t"
                          << p.byte_offset << "\t"
                          << p.byte_length << "\t"
                          << (p.has_filesystem ? "Y" : "N") << "\t"
                          << p.fs_type << "\n";
            }
        }
    }


    void displayTree();
    void extractFile();
}

#endif //DISK_ANALYSIS_WRAPPERS_HPP