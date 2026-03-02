//
// Created by rokas on 3/2/26.
//

#ifndef DISK_ANALYSIS_VOLUME_HPP
#define DISK_ANALYSIS_VOLUME_HPP

#include <memory>
#include <tsk/libtsk.h>
#include <nlohmann/json.hpp>

#include "disk.hpp"
#include "filesystem.hpp"

namespace da {
    struct PartitionInfo {
        uint32_t id;
        std::string name;

        TSK_OFF_T byte_offset;
        TSK_OFF_T byte_length;

        bool has_filesystem;
        std::string fs_type;

        // JSON serialization support
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(PartitionInfo, id, name, byte_offset, byte_length, has_filesystem, fs_type)
    };

    class Volume {
        TSK_VS_INFO* v_ = nullptr;
        const Disk* disk_ = nullptr;

    public:
        Volume(const Disk& disk) : disk_(&disk) {
            v_ = tsk_vs_open(disk.get(), 0, TSK_VS_TYPE_DETECT);
            if (!v_) throw std::runtime_error(tsk_error_get());
        }

        ~Volume() { if (v_) tsk_vs_close(v_); }

        [[nodiscard]]
        std::vector<PartitionInfo> getPartitions() const {
            std::vector<PartitionInfo> partitions;
            if (!v_) return partitions;

            for (TSK_PNUM_T i = 0; i < v_->part_count; i++) {
                const auto* partition = tsk_vs_part_get(v_, i);
                if (!partition) continue;

                // skip metadata parts (mbr, gpt tables)
                if (partition->flags & TSK_VS_PART_FLAG_META) continue;

                std::string fs_name = "None";

                // if partition is allocated
                if (partition->flags & TSK_VS_PART_FLAG_ALLOC) {
                    TSK_OFF_T offset = partition->start * v_->block_size;
                    auto* fs = tsk_fs_open_img(disk_->get(), offset, TSK_FS_TYPE_DETECT);

                    if (fs) {
                        fs_name = tsk_fs_type_toname(fs->ftype);
                        tsk_fs_close(fs);
                    } else {
                        fs_name = "Unknown";
                        tsk_error_reset();
                    }
                }

                partitions.push_back({
                    partition->addr,
                    partition->desc ? partition->desc : "Unknown",
                    static_cast<TSK_OFF_T>(partition->start * v_->block_size),
                    static_cast<TSK_OFF_T>(partition->len * v_->block_size),
                    partition->flags & TSK_VS_PART_FLAG_ALLOC ? true : false,
                    fs_name
                });
            }

            return partitions;
        };

        std::unique_ptr<FileSystem> openFS(uint32_t partitionId) {
            if (!v_) throw std::runtime_error("Volume not initialized");

            const TSK_VS_PART_INFO* partition = tsk_vs_part_get(v_, partitionId);
            if (!partition) throw std::runtime_error("Invalid partition ID");

            auto offset = static_cast<TSK_OFF_T>(partition->start * v_->block_size);
            return std::make_unique<FileSystem>(*disk_, offset);
        }
    };
}

#endif //DISK_ANALYSIS_VOLUME_HPP