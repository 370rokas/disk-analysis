//
// Created by rokas on 3/2/26.
//

#ifndef DISK_ANALYSIS_DEF_HPP
#define DISK_ANALYSIS_DEF_HPP

namespace da {
    enum class ActionType {
        Unknown,
        ListPartitions,
        Tree,
        ExtractFile,
        RunScript
    };
}

#endif //DISK_ANALYSIS_DEF_HPP