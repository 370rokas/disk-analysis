# disk-analysis (da)

A terminal tool for disk analysis and data extraction from file systems. Powered by The Sleuth Kit (TSK) with an emphasis on scriptable workflows.
## Usage

Docs coming soon!

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
git clone [https://github.com/yourusername/disk-analysis.git](https://github.com/yourusername/disk-analysis.git)
cd disk-analysis

# 2. Install dependencies via vcpkg
vcpkg install

# 3. Build the project
mkdir build && cd build
cmake ..
make

# 4. Run the tool
./disk-analysis [options]
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