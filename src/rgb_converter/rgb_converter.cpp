/*
 * AIM rgb_converter (0.03 demo textures files)
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

#include "mmap.h"

#include <primitives/sw/main.h>
#include <primitives/sw/settings.h>
#include <primitives/sw/cl.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdint.h>
#include <string>

#pragma pack(push, 1)
struct rgb {
    struct header {
        uint32_t magic;
        uint32_t width;
        uint32_t height;
        uint32_t type;
        uint32_t unk2;
        uint32_t unk3;
    };

    static void convert_model(const path &fn) {
        primitives::templates2::mmap_file<uint8_t> f{fn};
        stream s{f};

        header h = s;
        // maybe take something from tm_converter?
        if (memcmp(&h.magic, "SGTC", sizeof(h.magic) == 0)) {
            if (h.type == 1) {
                auto bytes = s.span<uint16_t>(h.width * h.height);
            }
            if (h.type == 6) {
                auto bytes = s.span<uint16_t>(h.width * h.height);
            }
        }
        if (memcmp(&h.magic, "SGTF", sizeof(h.magic) == 0)) {
            if (h.type == 2) {
                auto bytes = s.span<uint16_t>(h.width * h.height);
            }
        }

        int a = 5;
        a++;
    }
};
#pragma pack(pop)

int main(int argc, char *argv[])
{
    cl::opt<path> p(cl::Positional, cl::desc("<.rgb file or dir>"), cl::Required);

    cl::ParseCommandLineOptions(argc, argv);

    if (fs::is_regular_file(p)) {
        rgb::convert_model(p);
    } else if (fs::is_directory(p)) {
        auto files = enumerate_files(p, false);
        for (auto &f : FilesSorted(files.begin(), files.end()))
        {
            if (f.extension() != ".rgb") {
                continue;
            }
            std::cout << "processing: " << f << "\n";
            try
            {
                rgb::convert_model(f);
            }
            catch (std::exception &e)
            {
                std::cout << "error: " << e.what() << "\n";
            }
        }
    }
    else
        throw std::runtime_error("No such file or directory: " + to_printable_string(normalize_path(p)));

    return 0;
}
