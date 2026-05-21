//
// Created by rokas on 4/27/26.
//

#include "scripting/lua.hpp"

#include <stdexcept>

#include <sol/sol.hpp>

#include "actions/extract.hpp"
#include "actions/partitions.hpp"
#include "core/context.hpp"
#include "core/filesystem.hpp"
#include "core/fsEntry.hpp"
#include "core/volume.hpp"

namespace da {
    namespace {
        sol::table fsEntryChildren(FSEntry& entry, sol::this_state ts) {
            sol::state_view lua(ts);
            sol::table tbl = lua.create_table();
            for (const auto& [name, child] : entry.children()) {
                tbl[name] = child;
            }
            return tbl;
        }

        sol::table listPartitionsLua(sol::this_state ts) {
            sol::state_view lua(ts);
            auto& ctx = Ctx::get();
            auto parts = Partitions::listPartitions(ctx.getDisk());

            sol::table tbl = lua.create_table();
            int i = 1;
            for (const auto& p : parts) {
                tbl[i++] = p;
            }
            return tbl;
        }

        std::unique_ptr<FileSystem> openFsLua(uint32_t partition_id) {
            return Partitions::getFileSystem(Ctx::get().getDisk(), partition_id);
        }

        void registerBindings(sol::state& lua, const std::string& settings) {
            lua.open_libraries(
                sol::lib::base,
                sol::lib::package,
                sol::lib::string,
                sol::lib::table,
                sol::lib::math,
                sol::lib::io,
                sol::lib::os
            );

            lua.new_usertype<PartitionInfo>("PartitionInfo",
                sol::no_constructor,
                "id", &PartitionInfo::id,
                "name", &PartitionInfo::name,
                "byte_offset", &PartitionInfo::byte_offset,
                "byte_length", &PartitionInfo::byte_length,
                "has_filesystem", &PartitionInfo::has_filesystem,
                "fs_type", &PartitionInfo::fs_type
            );

            lua.new_usertype<FSEntry>("FSEntry",
                sol::no_constructor,
                "name", &FSEntry::name,
                "full_path", &FSEntry::fullPath,
                "size", &FSEntry::size,
                "is_directory", &FSEntry::isDirectory,
                "is_valid", &FSEntry::isValid,
                "is_link", &FSEntry::isLink,
                "link_target", &FSEntry::linkTarget,
                "load_all_descendants", &FSEntry::loadAllDescendants,
                "children", &fsEntryChildren
            );

            lua.new_usertype<FileSystem>("FileSystem",
                sol::no_constructor,
                "root", [](FileSystem& fs) -> FSEntry* { return fs.root(); },
                "extract", &da::extractFile
            );

            sol::table da_tbl = lua.create_named_table("da");

            auto& ctx = Ctx::get();
            da_tbl["image_path"] = ctx.config.image_path;
            da_tbl["settings"] = settings;

            da_tbl.set_function("list_partitions", &listPartitionsLua);
            da_tbl.set_function("open_fs", &openFsLua);

            da_tbl.set_function("log_info", [](const std::string& msg) {
                Ctx::get().getLogger().info(msg);
            });
            da_tbl.set_function("log_warn", [](const std::string& msg) {
                Ctx::get().getLogger().warn(msg);
            });
            da_tbl.set_function("log_error", [](const std::string& msg) {
                Ctx::get().getLogger().error(msg);
            });
        }
    } // anonymous

    void runLuaScript(const std::string& script_path, const std::string& settings) {
        sol::state lua;
        registerBindings(lua, settings);

        const auto result = lua.safe_script_file(script_path, &sol::script_pass_on_error);
        if (!result.valid()) {
            const sol::error err = result;
            throw std::runtime_error(std::string("Lua error: ") + err.what());
        }
    }
} // da
