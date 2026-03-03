//
// Created by rokas on 3/2/26.
//

#include "partitions.hpp"

std::vector<da::PartitionInfo> da::Partitions::listPartitions(const Disk &disk) {
    try {
        // Attempt to open the volume system
        const Volume vol(disk);
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

std::unique_ptr<da::FileSystem> da::Partitions::getFileSystem(const Disk &disk, uint32_t partition_id) {
    // try to read the volume, if it doesn't exist, try reading the whole disk as a FS
    try {
        Volume vol(disk);
        return vol.openFS(partition_id);
    } catch (const std::exception& e) {
        tsk_error_reset();
        return std::make_unique<FileSystem>(disk, 0);
    }
}
