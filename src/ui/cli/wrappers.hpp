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

    inline void treeFilesystem() {
        auto &ctx = Ctx::get();
        auto target_partition_id = ctx.config.target_partition;

        const auto FS = da::Partitions::getFileSystem(ctx.getDisk(), target_partition_id);

        if (!FS) {
            std::cerr << "Failed to get filesystem for partition ID: " << target_partition_id << std::endl;
            return;
        }

        auto root = FS->root();
        root->loadAllDescendants();

        if (ctx.config.json_output) {
            nlohmann::json j = *root;
            std::cout << j.dump() << std::endl;
        } else if (ctx.config.csv_output) {
            std::cout << "name,size,is_directory,linksTo\n";
            std::function<void(const da::FSEntry*)> printEntry;
            printEntry = [&](const da::FSEntry* entry) {
                std::cout << "\"" << entry->fullPath() << "\","
                          << entry->size() << ","
                          << (entry->isDirectory() ? "true" : "false") << ","
                          << (entry->isLink() ? entry->linkTarget() : "null")
                          << "\n";

                if (entry->isDirectory()) {
                    for (const auto& [name, child] : entry->children()) {
                        printEntry(child);
                    }
                }
            };
            printEntry(root);
        } else {
            std::function<void(const da::FSEntry*, int)> printEntry;
            printEntry = [&](const da::FSEntry* entry, int indent) {
                std::cout << std::string(indent * 2, ' ')
                          << entry->name() << " (size: " << entry->size() << ", "
                          << (entry->isDirectory() ? "dir" : "file") << ")\n";

                if (entry->isDirectory()) {
                    for (const auto& [name, child] : entry->children()) {
                        printEntry(child, indent + 1);
                    }
                }
            };
            printEntry(root, 0);
        }
    }

    void extractFile();
}

#endif //DISK_ANALYSIS_WRAPPERS_HPP