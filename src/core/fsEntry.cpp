//
// Created by rokas on 3/2/26.
//

#include "fsEntry.hpp"

#include <iostream>

namespace da {
    void FSEntry::loadChildren_() {
        if (areChildrenLoaded_ || !isDirectory_ || !handle_) return;

        dir_ = tsk_fs_dir_open_meta(handle_->fs_info, handle_->meta->addr);
        if (!dir_) return;

        const auto nEntries = tsk_fs_dir_getsize(dir_);
        for (auto i = 0; i < nEntries; i++) {
            TSK_FS_FILE* fsFile = tsk_fs_dir_get(dir_, i);
            if (fsFile == nullptr) continue;

            // Skip unallocated files
            if (fsFile->name->flags & TSK_FS_NAME_FLAG_UNALLOC) {
                tsk_fs_file_close(fsFile);
                continue;
            }

            // skip "." and ".." entries
            if (fsFile->name && (strcmp(fsFile->name->name, ".") == 0 || strcmp(fsFile->name->name, "..") == 0)) {
                tsk_fs_file_close(fsFile);
                continue;
            }

            // skip ntfs $OrphanFiles
            if (fsFile->name && strcmp(fsFile->name->name, "$OrphanFiles") == 0) {
                tsk_fs_file_close(fsFile);
                continue;
            }

            children_[fsFile->name->name] = new FSEntry(fsFile, this);
        }

        areChildrenLoaded_ = true;
    }

    void to_json(nlohmann::json &j, const FSEntry &entry) {
        j = nlohmann::json{
            {"name", entry.name()},
            {"size", entry.size()},
            {"is_directory", entry.isDirectory()}
        };

        if (entry.isDirectory()) {
            j["children"] = nlohmann::json::array();
            for (const auto &child: entry.children() | std::views::values) {
                nlohmann::json childJson;
                to_json(childJson, *child);
                j["children"].push_back(childJson);
            }
        }

        if (entry.isLink()) {
            j["is_link"] = true;
            j["link_target"] = entry.linkTarget();
        }
    }

    std::string FSEntry::name() const {
        if (!handle_ || !handle_->name || !handle_->name->name) {
            return {}; // Invalid handle or name
        }
        return { handle_->name->name };
    }

    void FSEntry::loadAllDescendants() {
        if (!isDirectory_) return;

        loadChildren_();
        for (const auto& [name, child] : children_) {
            child->loadAllDescendants();
        }
    }
} // da