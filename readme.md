# disk-analysis

A terminal tool for disk analysis and data extraction from file systems.

Requirements:
- Linux/Mac/Windows OS support.
- User interface via terminal. Data is presented in plain or JSON format.
- Support for scanning major file systems (EXT4, FAT32, possibly NTFS?). Combine different file system scanning libraries into a single abstraction layer.
- Functions: tree, exporting a specific file.
- LUA script support for analysis/data export automation.

The tool uses VCPKG for package management. 

## Structure

- `src/`: Source code for the tool.
  - `core/`: Core functionality, TSK wrappers, classes for file system abstraction.
  - `actions/`: Code for actions like scanning, exporting, etc.
  - `ui/`: User interface code - `cli` / `lua`.
- `scripts/`: LUA scripts for the tool.