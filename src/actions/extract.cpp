//
// Created by rokas on 5/21/26.
//

#include "actions/extract.hpp"

#include <fstream>
#include <vector>

namespace da {
    bool extractFile(FileSystem& fs, const std::string& in_path, const std::string& out_path) {
        TSK_FS_FILE* f = tsk_fs_file_open(fs.get(), nullptr, in_path.c_str());
        if (!f) {
            tsk_error_reset();
            return false;
        }

        std::ofstream out(out_path, std::ios::binary);
        if (!out) {
            tsk_fs_file_close(f);
            return false;
        }

        constexpr size_t BUF = 64 * 1024;
        std::vector<char> buf(BUF);

        const auto total = f->meta ? f->meta->size : 0;
        TSK_OFF_T offset = 0;
        while (offset < total) {
            const auto to_read = std::min<size_t>(BUF, total - offset);
            const auto n = tsk_fs_file_read(f, offset, buf.data(), to_read, TSK_FS_FILE_READ_FLAG_NONE);
            if (n <= 0) break;
            out.write(buf.data(), n);
            offset += n;
        }

        tsk_fs_file_close(f);
        return true;
    }
}
