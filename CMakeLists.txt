cmake_minimum_required(VERSION 3.10)
project(disk-analysis)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# find dependencies
find_package(TSK REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

# set sources
set(CODE_SOURCE_FILES
        src/main.cpp
        src/core/fsEntry.cpp
        src/actions/partitions.cpp
)

add_executable(disk-analysis
        ${CODE_SOURCE_FILES})

target_include_directories(disk-analysis PRIVATE
        src)

target_link_libraries(disk-analysis PRIVATE
        TSK::TSK
        CLI11::CLI11
        spdlog::spdlog)

# debug flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

# winbloat specific stuff
if(WIN32)
    target_compile_definitions(disk-analysis PRIVATE TSK_WIN32)
endif()