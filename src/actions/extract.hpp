//
// Created by rokas on 5/21/26.
//

#ifndef DISK_ANALYSIS_EXTRACT_HPP
#define DISK_ANALYSIS_EXTRACT_HPP

#include <string>

#include "core/filesystem.hpp"

namespace da {
    bool extractFile(FileSystem& fs, const std::string& in_path, const std::string& out_path);
}

#endif //DISK_ANALYSIS_EXTRACT_HPP
