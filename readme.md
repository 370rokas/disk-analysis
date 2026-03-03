# disk-analysis (da)

A terminal tool for disk analysis and data extraction from file systems. Powered by The Sleuth Kit (TSK) with an emphasis on scriptable workflows.
## Usage

Docs coming soon!

```bash
disk-analysis [options] <image> <subcommand> [args]

# Examples:

## See all available subcommands and options
disk-analysis --help

## List partitions on a disk image in CSV format
disk-analysis --csv images/blk0_mmcblk0.bin ls

## Get a tree view of the file system with id 30 in JSON format
disk-analysis --json images/blk0_mmcblk0.bin tree 30
```

## Prerequisites

Before building, ensure you have the following installed:

* **Build Tools:** [CMake](https://cmake.org/download/) and a C++ compiler.
* **Package Manager:** [VCPKG](https://learn.microsoft.com/en-us/vcpkg/get_started/overview).
* **Library:** **SleuthKit (TSK)**
  * **Linux:** `sudo apt install libtsk-dev` (or equivalent).
  * **macOS:** `brew install sleuthkit`.
  * **Windows:** Follow [compilation instructions](https://github.com/sleuthkit/sleuthkit/blob/develop/INSTALL.txt) or ensure binaries are in your `PATH`.

## Building

```bash
# 1. Clone the repository
git clone https://github.com/370rokas/disk-analysis.git
cd disk-analysis

# 2. Install dependencies via vcpkg
vcpkg install

# 3. Build the project
mkdir build && cd build
cmake ..
make

# 4. Run the tool
./disk-analysis (...)
   ```

## Structure

- `src/`: Source code for the tool.
  - `core/`: Core functionality, TSK wrappers, classes for file system abstraction.
  - `actions/`: Code for actions like scanning, exporting, etc.
  - `ui/`: User interface code - `cli` / `lua`.
- `scripts/`: LUA scripts for the tool.

## TODO:

- [X] Implement basic TSK wrappers.
- [X] Implement different output format support (human readable, CSV, JSON).
- [ ] Implement basic subcommands:
  - [X] `ls`: List files and directories.
  - [X] `tree`: Display file system tree.
  - [ ] `export`: Export a specific file.
- [ ] Implement LUA scripting support.
- [ ] Create documentation and usage examples.
- [ ] Automated testing for core functionality.
- [ ] Automated builds and releases.
- [ ] (idea): MCP server for AI agents.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.