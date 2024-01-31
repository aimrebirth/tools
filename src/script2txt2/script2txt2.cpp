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

#include <primitives/sw/main.h>
#include <primitives/sw/settings.h>
#include <primitives/sw/cl.h>

#pragma pack(push, 1)
struct script {
    uint32_t file_size;
    uint32_t unk0; // stack size? always 16000? // section bits?
    uint32_t raw_text_size;
    uint32_t nlines;
};
#pragma pack(pop)

int main(int argc, char *argv[]) {
    cl::opt<path> p(cl::Positional, cl::desc("<script.scr or scripts dir>"), cl::Required);

    cl::ParseCommandLineOptions(argc, argv);

    auto func = [](auto &&fn) {
        primitives::templates2::mmap_file<uint8_t> f{fn};
        stream s{f};
        script scr = s;
        auto text = s.span<uint8_t>(scr.raw_text_size);
        uint32_t max_nlines = s;
        auto rest = s.span<uint32_t>(max_nlines);
        std::vector<std::string_view> lines;
        lines.reserve(scr.nlines);
        for (int i = 0; i < scr.nlines; ++i) {
            lines.emplace_back((const char *)f.p + sizeof(script) + rest[i]);
        }
        int a = 5;
        a++;

        // write script
        /*{
            filename += ".txt";
            std::ofstream ofile(filename);
            if (ofile)
                ofile << ctx.getText();
        }

        // write function calls
        {
            std::ofstream functions("functions.txt", std::ios::app);
            if (functions)
            {
                for (auto &f : driver.functions)
                {
                    std::string f2(f.size(), 0);
                    std::transform(f.begin(), f.end(), f2.begin(), tolower);
                    functions << f2 << "\n";
                }
            }
        }*/
    };

    if (fs::is_regular_file(p)) {
        func(p.string());
    } else if (fs::is_directory(p)) {
        auto files = enumerate_files_like(p, ".*\\.scr", false);
        auto files2 = enumerate_files_like(p, ".*\\.QST", false);
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
