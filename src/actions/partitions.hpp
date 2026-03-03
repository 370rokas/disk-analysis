//
// Created by rokas on 3/2/26.
//

#ifndef DISK_ANALYSIS_PARTITIONS_HPP
#define DISK_ANALYSIS_PARTITIONS_HPP

#include "core/volume.hpp"

namespace da {
    class Partitions {
    public:
        static std::vector<PartitionInfo> listPartitions(const Disk& disk);
        static std::unique_ptr<FileSystem> getFileSystem(const Disk& disk, uint32_t partition_id);
    };
}

#endif //DISK_ANALYSIS_PARTITIONS_HPP