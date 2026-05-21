# Internals

This document describes how disk-analysis is structured and how each layer fits together. It is intended for contributors or anyone who wants to extend the tool.

---

## Architecture

The codebase is split into four layers. Each layer depends only on the layers below it:

```
┌─────────────────────────────────────────┐
│  ui/cli  ·  scripting                   │  presentation layer
├─────────────────────────────────────────┤
│  actions                                │  business logic
├─────────────────────────────────────────┤
│  core                                   │  TSK wrappers & domain types
├─────────────────────────────────────────┤
│  The Sleuth Kit (libtsk)                │  forensic I/O
└─────────────────────────────────────────┘
```

`main.cpp` sits above everything: it parses the CLI, builds the global context, and dispatches to either an action wrapper (for `ls`, `tree`, `extract`) or the Lua runtime (for `script`).

---

## Core layer (`src/core/`)

The core layer wraps the raw TSK C API in RAII objects and provides the shared domain types used throughout the rest of the codebase.

### `Disk` (`disk.hpp`)

Wraps a `TSK_IMG_INFO*`. Constructed from a filesystem path; the destructor calls `tsk_img_close`. All other core objects take a `const Disk&` reference rather than owning the image themselves.

```
Disk("disk.img")  →  TSK_IMG_INFO*
```

### `Volume` (`volume.hpp`)

Wraps a `TSK_VS_INFO*` (partition/volume system). Opens via `tsk_vs_open` with auto-detection. Provides:

- `getPartitions()` — iterates all TSK partitions, skips metadata entries (`TSK_VS_PART_FLAG_META`), probes each allocated partition for a filesystem to populate `fs_type`, and returns a `vector<PartitionInfo>`.
- `openFS(partitionId)` — fetches the partition by ID via `tsk_vs_part_get` and constructs a `FileSystem` at the correct byte offset.

`PartitionInfo` is a plain struct with JSON serialization via `NLOHMANN_DEFINE_TYPE_INTRUSIVE`.

### `FileSystem` (`filesystem.hpp`)

Wraps a `TSK_FS_INFO*`. Opened at a specific byte offset within the image. Owns a lazily-created root `FSEntry` — the first call to `root()` opens the root inode via `tsk_fs_file_open_meta` and allocates the entry.

### `FSEntry` (`fsEntry.hpp`, `fsEntry.cpp`)

Represents a single file or directory in the filesystem tree. This is the most complex core class.

**Construction** takes a `TSK_FS_FILE*` handle and a parent pointer. On construction, two things happen:

1. `isDirectory_` is set from the inode metadata type.
2. The inode address (`handle_->meta->addr`) is looked up in `Ctx::inodeDirMap`:
   - **First occurrence** — the address is stored with the entry's path. The entry is created normally.
   - **Subsequent occurrence** — the inode has already been mapped (hard link). The handle is set to `nullptr`, `isLink_` is set to `true`, and `linkTarget_` stores the path of the first occurrence. The entry becomes a lightweight placeholder.

**Lazy child loading** — `children()` is a `const` method that calls `loadChildren_()` via `const_cast` on first access. `loadChildren_()` opens the TSK directory handle, iterates all entries, and skips:
- Unallocated entries (`TSK_FS_NAME_FLAG_UNALLOC`)
- `.` and `..`
- NTFS `$OrphanFiles`

Each surviving entry becomes a heap-allocated `FSEntry*` stored in `children_` (a `map<string, FSEntry*>` keyed by name). The parent owns all children and deletes them in its destructor.

**`loadAllDescendants()`** — recursively calls `loadChildren_()` across the entire subtree. Used before JSON/CSV output, which must traverse the complete tree in one pass.

**`fullPath()`** — walks parent pointers to reconstruct the absolute filesystem path. The root entry has no parent, so its path is just its name (an empty string when opened via `tsk_fs_file_open_meta`), producing paths like `/etc/passwd`.

**JSON serialisation** (`to_json`) emits `name`, `size`, `is_directory`, an optional `children` array (for directories), and optional `is_link` + `link_target` (for hard link placeholders).

### `Context` (`context.hpp`)

A global singleton (`Ctx::get()`) that holds shared state across the entire run:

| Field | Type | Purpose |
|---|---|---|
| `config` | `CliConfig` | Parsed CLI options |
| `disk` | `unique_ptr<Disk>` | The open disk image |
| `inodeDirMap` | `map<TSK_INUM_T, string>` | Inode → first-seen path, used for hard link detection |

The logger is also initialised lazily through the context (`getLogger()`), creating spdlog sinks based on `config.log_file` and `config.log_console` on first access.

---

## Actions layer (`src/actions/`)

Thin stateless functions that orchestrate core objects to perform a complete operation. They do not know about output format — that is the CLI layer's job.

### `Partitions` (`partitions.hpp`, `partitions.cpp`)

- `listPartitions(disk)` — tries `Volume(disk).getPartitions()`. If that throws (no recognisable partition table — e.g. a USB stick formatted as a bare filesystem), falls back to probing the whole disk as a single filesystem and returns a synthetic `PartitionInfo` with `id=0`, `name="Whole Disk"`.
- `getFileSystem(disk, partition_id)` — tries `Volume(disk).openFS(partition_id)`. Falls back to `FileSystem(disk, 0)` if no volume system is found.

The fallback behaviour means the tool works correctly on raw filesystem images with no partition table.

### `Extract` (`extract.hpp`, `extract.cpp`)

`extractFile(fs, in_path, out_path)` — opens the file at `in_path` within `fs` using `tsk_fs_file_open`, then streams it to `out_path` in 64 KB chunks via `tsk_fs_file_read`. Returns `true` on success.

This function is the single source of extraction logic — it is called both by the CLI wrapper and exposed to Lua via the `FileSystem` usertype binding.

---

## CLI layer (`src/ui/cli/`)

### `CliParser` and `CliConfig` (`cli.hpp`)

Uses the [CLI11](https://github.com/CLIUtils/CLI11) library. `CliConfig` is a plain struct that `CliParser` fills in during `parse()`. The result is stored into `Ctx::get().config` in `main.cpp`.

Subcommands registered:

| Subcommand | Required args | Sets `config.action` to |
|---|---|---|
| `ls` | — | `ListPartitions` |
| `tree` | `partition` | `Tree` |
| `extract` | `partition`, `in_path`, `out_path` | `ExtractFile` |
| `script` | `in_path`, optional `settings` | `RunScript` |

`--json` and `--csv` are added to an option group with `excludes()` between them, so CLI11 enforces mutual exclusion.

### Wrappers (`wrappers.hpp`)

Inline functions in `da::cli` that read from `Ctx::get().config` and produce output in the requested format. Each wrapper follows the same pattern:

```
read config → call action → branch on json_output / csv_output / default → print to stdout
```

`extractFile()` wrapper: opens the filesystem, calls `da::extractFile`, prints an error to `stderr` on failure. No stdout output on success.

---

## Scripting layer (`src/scripting/`)

### `runLuaScript` (`lua.hpp`, `lua.cpp`)

Creates a `sol::state`, calls `registerBindings`, then runs the script file via `lua.safe_script_file`. Any Lua error is converted to a `std::runtime_error` which propagates up to `main.cpp`'s catch block.

### `registerBindings`

Exposes the full API to Lua:

**Usertypes** (C++ classes usable directly in Lua):

- `PartitionInfo` — read-only field access for all six fields.
- `FSEntry` — all methods from the public API including `children()` (adapted to return a Lua table instead of a C++ map).
- `FileSystem` — `root()` and `extract()` (the latter calls `da::extractFile` directly).

**`da` table** — the primary scripting entry point:

```lua
da.image_path          -- string: path to the disk image
da.settings            -- string: settings argument from the command line
da.list_partitions()   -- → table of PartitionInfo
da.open_fs(id)         -- → FileSystem
da.log_info(msg)       -- → void
da.log_warn(msg)       -- → void
da.log_error(msg)      -- → void
```

Standard Lua libraries available: `base`, `package`, `string`, `table`, `math`, `io`, `os`.

---

## Execution flow

```
main()
 ├── CliParser::parse()          builds CliConfig
 ├── Ctx::get().config = config
 ├── Ctx::get().disk = Disk(image_path)
 └── switch(action)
      ├── ListPartitions  →  da::cli::listPartitions()
      │                        └── Partitions::listPartitions(disk)
      │                              └── Volume::getPartitions()  [or fallback]
      ├── Tree            →  da::cli::treeFilesystem()
      │                        └── Partitions::getFileSystem(disk, id)
      │                              └── root()->loadAllDescendants()
      ├── ExtractFile     →  da::cli::extractFile()
      │                        └── Partitions::getFileSystem(disk, id)
      │                              └── da::extractFile(fs, in, out)
      └── RunScript       →  da::runLuaScript(path, settings)
                               └── [Lua script executes, calls da.* API]
```

---

## Key design decisions

### Raw disk fallback

`Partitions::listPartitions` and `Partitions::getFileSystem` both catch exceptions from `Volume` construction and silently fall back to treating the whole image as a single filesystem. This means the tool works on raw filesystem images (`.ext4`, `.ntfs`, etc.) without a partition table, producing a single synthetic entry with `id=0`.

### Hard link detection

TSK can walk the same inode multiple times when a file has more than one directory entry (hard link). Without deduplication, a recursive tree walk could follow circular links infinitely.

`Ctx::inodeDirMap` acts as a visited set keyed by inode address. On the first visit, the address is recorded with the entry's path. On any subsequent visit, `FSEntry` sets `handle_ = nullptr` and `isLink_ = true`. Because `isLink_` entries have a null handle, they cannot be opened, read, or descended into — they simply record where to look.

The map is never cleared between operations. This is intentional: a single run only has one disk/filesystem context, so the inode space is stable for the lifetime of the process.

### Lazy child loading

`FSEntry::children()` is `const` but uses `const_cast` to call `loadChildren_()` on first access. This pattern avoids loading the full tree upfront for operations (like `extract`) that only need to open a single file by path, while still allowing `loadAllDescendants()` to eagerly populate the tree for tree-walk operations.

### Shared extraction logic

`da::extractFile` lives in `actions/extract.cpp` and is called from both `da::cli::extractFile()` (the CLI handler) and the Lua `FileSystem::extract` binding. Neither the CLI nor the Lua layer contain any extraction logic themselves — both delegate to the same function.

---

## Dependencies

| Library | Purpose | Managed by |
|---|---|---|
| [libtsk](https://www.sleuthkit.org/) | Filesystem and disk image parsing | System (`apt`/`brew`) |
| [CLI11](https://github.com/CLIUtils/CLI11) | Command-line argument parsing | vcpkg |
| [nlohmann/json](https://github.com/nlohmann/json) | JSON serialisation | vcpkg |
| [spdlog](https://github.com/gabime/spdlog) | Logging | vcpkg |
| [Lua](https://www.lua.org/) | Scripting runtime | vcpkg |
| [sol2](https://github.com/ThePhD/sol2) | C++/Lua binding layer | vcpkg |
