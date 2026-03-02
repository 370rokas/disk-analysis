cmake_minimum_required(VERSION 3.10)
project(disk-analysis)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# find dependencies
find_package(TSK REQUIRED)
find_package(CLI11 CONFIG REQUIRED)

# set sources
set(CODE_SOURCE_FILES
        src/main.cpp
        src/ui/cli/cli.hpp
        src/core/context.hpp
        src/core/disk.hpp
        src/core/filesystem.hpp
        src/core/fsEntry.cpp
        src/core/fsEntry.hpp
        src/core/volume.hpp
        src/defs.hpp
        src/actions/partitions.cpp
        src/actions/partitions.hpp
        src/ui/cli/wrappers.hpp
)

add_executable(disk-analysis
        ${CODE_SOURCE_FILES})

target_include_directories(disk-analysis PRIVATE
        src)

target_link_libraries(disk-analysis PRIVATE
        TSK::TSK
        CLI11::CLI11)

# winbloat specific stuff
if(WIN32)
    target_compile_definitions(disk-analysis PRIVATE TSK_WIN32)
endif()