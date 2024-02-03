/*
 * AIM script2txt2 (simpler version)
 * Copyright (C) 2024 lzwdgc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <mmap.h>
#include <types.h>

#include <primitives/sw/main.h>
#include <primitives/sw/settings.h>
#include <primitives/sw/cl.h>

#include <fstream>

int main(int argc, char *argv[]) {
    cl::opt<path> p(cl::Positional, cl::desc("<script.scr or scripts dir>"), cl::Required);

    cl::ParseCommandLineOptions(argc, argv);

    auto func = [](path filename) {
        uint32_t sz{};
        auto lines = read_lines(filename);
        for (auto &&line : lines) {
            sz += line.size() + 1;
        }
        uint32_t maxlines = lines.size();

        uint32_t unk{};
        auto fsz = sizeof(script) + sz + maxlines * sizeof(sz) + sizeof(unk);

        primitives::templates2::mmap_file<uint8_t> f{filename.stem(), primitives::templates2::mmap_file<uint8_t>::rw{}};
        f.alloc_raw(fsz);
        stream s{f};
        script scr{};
        scr.nlines = lines.size();
        scr.raw_text_size = sz;
        scr.file_size = fsz - sizeof(scr.file_size);
        s = scr;

        for (auto &&line : lines) {
            boost::to_upper(line); // can be bad?
            boost::trim(line);
            boost::replace_all(line, " ", ""); // can be bad?
            memcpy((char *)s.p, line.c_str(), line.size() + 1);
            s.skip(line.size() + 1);
        }

        s = maxlines;
        uint32_t offset{};
        for (auto &&line : lines) {
            s = offset;
            offset += line.size() + 1;
        }
        s = offset;
    };

    if (fs::is_regular_file(p)) {
        func(p.string());
    } else if (fs::is_directory(p)) {
        auto files = enumerate_files_like(p, ".*\\.scr.txt", false);
        auto files2 = enumerate_files_like(p, ".*\\.QST.txt", false);
        files.insert(files2.begin(), files2.end());
        for (auto &f : files) {
            std::cout << "processing: " << f << "\n";
            func(f.string());
        }
    } else {
        throw std::runtime_error("Bad fs object");
    }

    return 0;
}
