//
// Created by rokas on 3/2/26.
//

#ifndef DISK_ANALYSIS_FILESYSTEM_HPP
#define DISK_ANALYSIS_FILESYSTEM_HPP

#include "disk.hpp"
#include "fsEntry.hpp"

namespace da {
    class FileSystem {
        TSK_FS_INFO* fs_ = nullptr;
        FSEntry* root_ = nullptr;

    public:
        FileSystem(const Disk& disk, TSK_OFF_T byte_offset = 0) {
            fs_ = tsk_fs_open_img(disk.get(), byte_offset, TSK_FS_TYPE_DETECT);

            if (!fs_) {
                throw std::runtime_error("Failed to open FS: " + std::string(tsk_error_get()));
            }
        }

        ~FileSystem() {
            if (root_) delete root_;
            if (fs_) tsk_fs_close(fs_);
        }

        [[nodiscard]]
        TSK_FS_INFO* get() const { return fs_; }

        [[nodiscard]]
        FSEntry* root() {
            if (!root_) {
                TSK_FS_FILE* rootFile = tsk_fs_file_open_meta(fs_, nullptr, fs_->root_inum);
                if (!rootFile) {
                    throw std::runtime_error("Failed to open root FS entry: " + std::string(tsk_error_get()));
                }
                root_ = new FSEntry(rootFile, nullptr);
            }
            return root_;
        }
    };
}

#endif //DISK_ANALYSIS_FILESYSTEM_HPP