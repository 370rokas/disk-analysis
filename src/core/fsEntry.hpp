//
// Created by rokas on 3/2/26.
//

#ifndef DISK_ANALYSIS_FSENTRY_HPP
#define DISK_ANALYSIS_FSENTRY_HPP

#include <map>
#include <ranges>
#include <string>
#include <nlohmann/json.hpp>

#include <tsk/libtsk.h>

#include "context.hpp"

namespace da {
    class FSEntry {
        TSK_FS_FILE* handle_ = nullptr;
        TSK_FS_DIR* dir_ = nullptr;

        FSEntry* parent_ = nullptr;

        bool isDirectory_ = false;
        bool areChildrenLoaded_ = false;
        std::map<std::string, FSEntry*> children_; // name -> entry

        bool isLink_ = false;
        std::string linkTarget_;

        void loadChildren_();
    public:
        FSEntry(TSK_FS_FILE* f, FSEntry* p) : handle_(f), parent_(p) {
            if (handle_ && handle_->meta) {
                isDirectory_ = (handle_->meta->type == TSK_FS_META_TYPE_DIR);
            }

            auto& ctx = Ctx::get();
            if (ctx.inodeDirMap.contains(handle_->meta->addr)) {
                // this inode has already been visited, so it's a hard link, use the mapped path
                linkTarget_ = ctx.inodeDirMap[handle_->meta->addr];
                isLink_ = true;
                handle_ = nullptr;
            } else {
                std::string fullPath = parent_ ? parent_->name() + "/" + name() : name();
                ctx.inodeDirMap[handle_->meta->addr] = fullPath;
            }
        }

        ~FSEntry() {
            for (auto& [name, child] : children_) {
                delete child;
            }
            children_.clear();

            if (dir_) {
                tsk_fs_dir_close(dir_);
                dir_ = nullptr;
            }

            if (handle_) {
                tsk_fs_file_close(handle_);
                handle_ = nullptr;
            }
        }

        [[nodiscard]] const std::map<std::string, FSEntry*>& children() const {
            const_cast<FSEntry*>(this)->loadChildren_();
            return children_;
        }
        [[nodiscard]] std::string name() const;
        [[nodiscard]] std::string fullPath() const {
            if (parent_) {
                return parent_->fullPath() + "/" + name();
            }
            return name();
        }
        [[nodiscard]] uint64_t size() const { return handle_->meta ? handle_->meta->size : 0; }

        [[nodiscard]] bool isDirectory() const { return isDirectory_; }
        [[nodiscard]] bool isValid() const { return handle_ != nullptr; }

        [[nodiscard]] bool isLink() const { return isLink_; }
        [[nodiscard]] std::string linkTarget() const { return linkTarget_; }

        void loadAllDescendants();
    };

    void to_json(nlohmann::json& j, const FSEntry& entry);
} // da

#endif //DISK_ANALYSIS_FSENTRY_HPP