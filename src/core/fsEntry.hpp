//
// Created by rokas on 3/2/26.
//

#ifndef DISK_ANALYSIS_FSENTRY_HPP
#define DISK_ANALYSIS_FSENTRY_HPP

#include <string>
#include <vector>

#include <tsk/libtsk.h>

namespace da {
    class FSEntry {
        TSK_FS_FILE* handle_;

    public:
        bool isDirectory = false;

        FSEntry(TSK_FS_FILE* f) : handle_(f) {
            if (handle_ && handle_->meta) {
                isDirectory = (handle_->meta->type == TSK_FS_META_TYPE_DIR);
            }
        }

        ~FSEntry() { if (handle_) tsk_fs_file_close(handle_); }

        [[nodiscard]]
        std::vector<FSEntry> getChildren() const;

        [[nodiscard]]
        std::string name() const;

        [[nodiscard]]
        uint64_t size() const { return handle_->meta ? handle_->meta->size : 0; }
    };
} // da

#endif //DISK_ANALYSIS_FSENTRY_HPP