//
// Created by rokas on 3/2/26.
//

#ifndef DISK_ANALYSIS_DISK_HPP
#define DISK_ANALYSIS_DISK_HPP

#include <stdexcept>

#include <tsk/libtsk.h>

namespace da {
    class Disk {
        TSK_IMG_INFO* img_ = nullptr;

    public:
        Disk(const std::string& path) {
            const TSK_TCHAR* paths[] = { path.c_str() };
            img_ = tsk_img_open(1, paths, TSK_IMG_TYPE_DETECT, 0);

            if (!img_) {
                throw std::runtime_error(tsk_error_get());
            }
        }

        ~Disk() { if (img_) tsk_img_close(img_); }

        [[nodiscard]]
        bool isValid() const { return img_ != nullptr; }

        [[nodiscard]]
        TSK_IMG_INFO* get() const { return img_; }
    };
}

#endif //DISK_ANALYSIS_DISK_HPP