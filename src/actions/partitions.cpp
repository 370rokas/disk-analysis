//
// Created by rokas on 3/2/26.
//

#include "partitions.hpp"

std::vector<da::PartitionInfo> da::Partitions::listPartitions(const Disk &disk) {
    try {
        // Attempt to open the volume system
        Volume vol(disk);
        return vol.getPartitions();
    } catch (const std::exception& e) {
        std::vector<PartitionInfo> partitions;
        tsk_error_reset();

        // attempt to treat the disk as one fs (usb stick, etc)
        auto* fs = tsk_fs_open_img(disk.get(), 0, TSK_FS_TYPE_DETECT);
        if (fs) {
            partitions.push_back({
                0,
                "Whole Disk",
                0,
                disk.get()->size,
                true,
                tsk_fs_type_toname(fs->ftype)
            });
        }

        return partitions;
    }
}
