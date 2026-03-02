//
// Created by rokas on 3/2/26.
//

#include "fsEntry.hpp"

#include <cstring>

namespace da {
    std::vector<da::FSEntry> da::FSEntry::getChildren() const {
        std::vector<da::FSEntry> children;

        TSK_FS_DIR* dir = tsk_fs_dir_open_meta(handle_->fs_info, handle_->meta->addr);
        if (!dir) return children;

        const auto nEntries = tsk_fs_dir_getsize(dir);
        for (size_t i = 0; i < nEntries; i++) {

            TSK_FS_FILE* fsFile = tsk_fs_dir_get(dir, i);
            if (!fsFile) continue;

            // skip "." and ".." entries
            if (fsFile->name && (strcmp(fsFile->name->name, ".") == 0 || strcmp(fsFile->name->name, "..") == 0)) {
                tsk_fs_file_close(fsFile);
                continue;
            }

            // no need to create a FSEntry explicitly, cause the constructor gets called automatically
            children.emplace_back(fsFile);

            tsk_fs_file_close(fsFile);
        }

        tsk_fs_dir_close(dir);
        return children;
    }

    std::string FSEntry::name() const {
        if (!handle_ || !handle_->name || !handle_->name->name) {
            return {}; // Invalid handle or name
        }
        return { handle_->name->name };
    }
} // da