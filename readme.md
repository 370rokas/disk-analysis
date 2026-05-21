# disk-analysis (da)

A terminal tool for disk image analysis and file extraction. Built on [The Sleuth Kit (TSK)](https://www.sleuthkit.org/) with support for human-readable, CSV, and JSON output and a Lua scripting engine for automated workflows.

## Quick Start

```bash
# List partitions on a disk image
disk-analysis disk.img ls

# Display the file tree of partition 1
disk-analysis disk.img tree 1

# Extract a file from partition 1
disk-analysis disk.img extract 1 /etc/passwd ./passwd.txt

# Run a Lua script against the image
disk-analysis disk.img script ./scripts/largest_files.lua
```

## Usage

```
disk-analysis [options] <image> <subcommand> [args]
```

`<image>` is a path to a disk image file (`.img`, `.bin`, `.dd`, etc.) or a physical device node. It must exist and be readable.

### Global options

| Flag | Description |
|---|---|
| `-j, --json` | Output in JSON format |
| `-c, --csv` | Output in CSV format |
| `-l, --log <file>` | Write log messages to a file |
| `--lc, --console` | Print log messages to the console |

`--json` and `--csv` are mutually exclusive. The default output is human-readable text.

---

### `ls` — List partitions

```
disk-analysis [options] <image> ls
```

Lists every non-metadata partition found in the image. If no partition table is detected (e.g. a raw USB stick formatted as a single filesystem), the entire disk is shown as one entry with ID `0`.

**Human-readable output:**
```
ID    Name             Byte Offset    Byte Length    Has Filesystem    Filesystem Type
1     Linux (0x83)     1048576        10737418240    Y                 ext4
2     Linux swap       10738466816    2147483648     N                 None
```

**JSON output** (`--json`):
```json
[
  {
    "id": 1,
    "name": "Linux (0x83)",
    "byte_offset": 1048576,
    "byte_length": 10737418240,
    "has_filesystem": true,
    "fs_type": "ext4"
  },
  {
    "id": 2,
    "name": "Linux swap",
    "byte_offset": 10738466816,
    "byte_length": 2147483648,
    "has_filesystem": false,
    "fs_type": "None"
  }
]
```

**CSV output** (`--csv`):
```
id,name,byte_offset,byte_length,has_filesystem,fs_type
1,"Linux (0x83)",1048576,10737418240,true,"ext4"
2,"Linux swap",10738466816,2147483648,false,"None"
```

---

### `tree` — Display filesystem tree

```
disk-analysis [options] <image> tree <partition>
```

Recursively walks the filesystem on `<partition>` and prints every file and directory. Use the ID from `ls` for `<partition>`.

Unallocated entries, `.`, `..`, and NTFS `$OrphanFiles` are excluded. Hard-linked inodes are shown once; subsequent appearances are displayed as links pointing to the first occurrence.

**Human-readable output:**
```
/ (size: 0, dir)
  bin (size: 4096, dir)
    bash (size: 1234576, file)
    ls (size: 147912, file)
  etc (size: 4096, dir)
    hostname (size: 12, file)
    passwd (size: 2048, file)
  usr (size: 4096, dir)
    bin (size: 4096, dir) → [link: /bin]
```

**JSON output** (`--json`):
```json
{
  "name": "",
  "size": 0,
  "is_directory": true,
  "children": [
    {
      "name": "etc",
      "size": 4096,
      "is_directory": true,
      "children": [
        {
          "name": "passwd",
          "size": 2048,
          "is_directory": false
        }
      ]
    },
    {
      "name": "lib",
      "size": 0,
      "is_directory": true,
      "is_link": true,
      "link_target": "/usr/lib"
    }
  ]
}
```

**CSV output** (`--csv`) — the `name` column contains the full path:
```
name,size,is_directory,linksTo
"/etc",4096,true,null
"/etc/passwd",2048,false,null
"/lib",0,true,/usr/lib
```

---

### `extract` — Extract a file

```
disk-analysis <image> extract <partition> <src-path> <dest-path>
```

Extracts a single file from `<partition>` at `<src-path>` (absolute path within the filesystem) and writes it to `<dest-path>` on the host. Output format flags have no effect on this subcommand.

```bash
# Extract /etc/shadow from partition 1
disk-analysis disk.img extract 1 /etc/shadow ./shadow.txt

# Extract a binary from an NTFS image
disk-analysis disk.img extract 2 /Windows/System32/cmd.exe ./cmd.exe
```

---

### `script` — Run a Lua script

```
disk-analysis <image> script <script.lua> [settings]
```

Executes a Lua script against the loaded image. The optional `settings` string is passed as-is to the script via `da.settings` and can carry arbitrary configuration (paths, flags, etc.).

```bash
# Run a script
disk-analysis disk.img script ./scripts/carve_images.lua

# Run a script with a settings string
disk-analysis disk.img script ./scripts/export.lua "out=/tmp/export,ext=jpg"
```

---

## Lua Scripting

Scripts have access to a `da` global table that exposes the image and TSK filesystem API.

### Global: `da`

| Name | Type | Description |
|---|---|---|
| `da.image_path` | string | Path to the disk image passed on the command line |
| `da.settings` | string | The optional settings argument passed after the script path |
| `da.list_partitions()` | function | Returns a table (array) of `PartitionInfo` objects |
| `da.open_fs(id)` | function | Opens the filesystem on partition `id`, returns a `FileSystem` |
| `da.log_info(msg)` | function | Log at INFO level |
| `da.log_warn(msg)` | function | Log at WARN level |
| `da.log_error(msg)` | function | Log at ERROR level |

### `PartitionInfo`

Fields are read-only. Obtain instances via `da.list_partitions()`.

| Field | Type | Description |
|---|---|---|
| `.id` | integer | Partition ID (use this with `da.open_fs` and `tree`/`extract`) |
| `.name` | string | Partition description from the partition table |
| `.byte_offset` | integer | Start of the partition in bytes |
| `.byte_length` | integer | Size of the partition in bytes |
| `.has_filesystem` | boolean | Whether a recognisable filesystem was found |
| `.fs_type` | string | Filesystem type name (e.g. `"ext4"`, `"ntfs"`, `"fat32"`) |

### `FileSystem`

Obtain via `da.open_fs(id)`.

| Method | Returns | Description |
|---|---|---|
| `fs:root()` | `FSEntry` | Root directory entry of the filesystem |
| `fs:extract(src, dest)` | boolean | Extract file at `src` (absolute FS path) to `dest` on the host |

### `FSEntry`

| Method | Returns | Description |
|---|---|---|
| `entry:name()` | string | File or directory name |
| `entry:full_path()` | string | Absolute path within the filesystem (e.g. `/etc/passwd`) |
| `entry:size()` | integer | Size in bytes (0 for directories) |
| `entry:is_directory()` | boolean | |
| `entry:is_valid()` | boolean | `false` if this entry is a hard link placeholder |
| `entry:is_link()` | boolean | `true` for hard-linked inodes after the first occurrence |
| `entry:link_target()` | string | Path of the first occurrence (only meaningful when `is_link()` is true) |
| `entry:load_all_descendants()` | — | Recursively loads all children into memory |
| `entry:children()` | table | `{ name → FSEntry }` map of direct children (triggers lazy load) |

### Example script

```lua
-- Export all .jpg files from every partition to /tmp/export/
local partitions = da.list_partitions()

for _, p in ipairs(partitions) do
    if not p.has_filesystem then goto continue end

    da.log_info(string.format("Scanning partition %d (%s)", p.id, p.fs_type))
    local ok, fs = pcall(da.open_fs, p.id)
    if not ok then goto continue end

    local root = fs:root()
    root:load_all_descendants()

    local function walk(entry)
        if entry:is_link() or not entry:is_valid() then return end

        if entry:is_directory() then
            for _, child in pairs(entry:children()) do
                walk(child)
            end
        elseif entry:name():match("%.jpg$") then
            local dest = "/tmp/export" .. entry:full_path()
            if fs:extract(entry:full_path(), dest) then
                da.log_info("Exported: " .. entry:full_path())
            else
                da.log_warn("Failed:   " .. entry:full_path())
            end
        end
    end

    walk(root)
    ::continue::
end
```

---

## Logging

Logging is off by default. Enable it with either flag — both can be used together:

```bash
# Log to a file
disk-analysis -l analysis.log disk.img tree 1

# Log to console
disk-analysis --console disk.img ls

# Log to both
disk-analysis -l analysis.log --console disk.img script carve.lua
```

---

## Prerequisites

Before building, ensure you have the following installed:

* **Build Tools:** [CMake](https://cmake.org/download/) (3.10+) and a C++20 compiler.
* **Package Manager:** [VCPKG](https://learn.microsoft.com/en-us/vcpkg/get_started/overview).
* **Library:** **SleuthKit (TSK)**
  * **Linux:** `sudo apt install libtsk-dev`
  * **macOS:** `brew install sleuthkit`
  * **Windows:** Follow the [compilation instructions](https://github.com/sleuthkit/sleuthkit/blob/develop/INSTALL.txt).

## Building

```bash
# 1. Clone the repository
git clone https://github.com/370rokas/disk-analysis.git
cd disk-analysis

# 2. Install vcpkg dependencies
vcpkg install

# 3. Configure and build
cmake -B build
cmake --build build

# 4. Run
./build/disk-analysis --help
```

To build with AddressSanitizer enabled (development only):
```bash
cmake -B build -DENABLE_ASAN=ON
cmake --build build
```

## Project Structure

```
src/
├── main.cpp              # Entry point — parses CLI and dispatches to actions
├── defs.hpp              # ActionType enum
├── core/                 # TSK wrappers and domain types
│   ├── disk.hpp          # Disk image (TSK_IMG_INFO)
│   ├── volume.hpp        # Partition table (TSK_VS_INFO) + PartitionInfo
│   ├── filesystem.hpp    # Filesystem handle (TSK_FS_INFO)
│   ├── fsEntry.hpp/cpp   # File/directory entry with lazy child loading
│   ├── context.hpp       # Global singleton — config, disk handle, inode map
│   └── logger.hpp        # spdlog initialisation
├── actions/              # Business logic
│   ├── partitions.hpp/cpp  # Partition listing and filesystem access
│   └── extract.hpp/cpp     # File extraction (shared by CLI and Lua)
├── ui/cli/               # Command-line interface
│   ├── cli.hpp           # CLI11 parser + CliConfig struct
│   └── wrappers.hpp      # Output formatters for each subcommand
└── scripting/            # Lua integration
    ├── lua.hpp/cpp       # sol2 bindings and script runner
scripts/                  # Example Lua scripts
```

## TODO

- [X] Implement basic TSK wrappers.
- [X] Implement different output format support (human readable, CSV, JSON).
- [X] Implement LUA scripting support (`da.list_partitions`, `da.open_fs`, `da.extract`, logging bindings).
- [X] Implement basic subcommands:
  - [X] `ls`: List partitions.
  - [X] `tree`: Display file system tree.
  - [X] `extract`: Extract a specific file.
- [X] Create documentation and usage examples.
- [ ] Automated testing for core functionality.
- [X] Automated builds and releases.
- [ ] (idea): MCP server for AI agents.
- [ ] (idea): Interactive TUI with live filesystem browsing.
- [ ] (idea): Support for Windows Registry hives and other non-filesystem data structures.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
