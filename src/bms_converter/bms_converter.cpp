/*
 * AIM bms_converter (0.03 demo binary model files)
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
struct bms {
    struct header {
        uint32_t magic;
        uint32_t version;
        uint32_t n_blocks; // anims prob
        uint32_t unk;
    };
    struct unk {
        uint32_t unk1;
        uint32_t n_unk;
        uint32_t unk2;
    };
    struct desc {
        uint32_t unk0;
        uint32_t unk1;
        uint32_t n_vertices;
        uint32_t unk2;
        uint32_t n_faces;
        uint32_t n_unk;
    };
    struct v {
        float x,y,z;
    };
    struct vn {
        float x, y, z;
    };
    struct vt {
        float x, y;
    };
    struct f {
        uint16_t x,y;
    };
    struct unk2 {
        uint16_t id;
        uint32_t unk1;
        float unk2[2];
        uint32_t unk3;
        float unk4[2];
    };

    using charC = char[0xC];

    int n_blocks;
    char header_[0x40];
};
#pragma pack(pop)

void convert_model(const path &fn) {
    primitives::templates2::mmap_file<uint8_t> f{fn};
    stream s{f};
    bms::header h = s;
    auto names = s.span<bms::charC>(h.n_blocks);
    uint32_t unk1 = s;
    auto unk_descs = s.span<bms::unk>(h.n_blocks);
    uint32_t n_vertices = s;
    uint32_t n_faces = s;
    //bms::desc d = s;
    auto vcs = s.span<bms::v>(n_vertices);
    auto vns = s.span<bms::vn>(n_vertices);
    auto vts = s.span<bms::vt>(n_vertices);
    auto vfs = s.span<uint16_t>(n_faces);
    for (auto &&d : unk_descs) {
        auto unk2 = s.span<bms::unk2>(d.n_unk);
        auto unk3 = s.span<bms::unk2>(d.unk2);
    }

    std::ofstream ofile{path{fn} += ".obj"};
    for (auto &&v : vcs) {
        ofile << std::format("v {} {} {}\n", v.x, v.y, v.z);
    }
    ofile << "\n";
    for (auto &&v : vns) {
        ofile << std::format("vn {} {} {}\n", v.x, v.y, v.z);
    }
    ofile << "\n";
    for (auto &&v : vts) {
        ofile << std::format("vt {} {}\n", v.x, v.y);
    }
    ofile << "\n";
    ofile << "g obj\n";
    for (auto &&f : std::views::chunk(vfs, 3)) {
        ofile << std::format("f {} {} {}\n", f[0], f[1], f[2]);
        /*ofile << std::format("f {}/{}/{} {}/{}/{} {}/{}/{}\n"
            , f[0], f[0], f[0]
            , f[1], f[1], f[1]
            , f[2], f[2], f[2]
        );*/
    }
    ofile << "g\n";
}

int main(int argc, char *argv[])
{
    cl::opt<path> p(cl::Positional, cl::desc("<mod file>"), cl::Required);

    cl::ParseCommandLineOptions(argc, argv);

    if (fs::is_regular_file(p)) {
        convert_model(p);
    } else if (fs::is_directory(p)) {
        auto files = enumerate_files(p, false);
        for (auto &f : FilesSorted(files.begin(), files.end()))
        {
            if (f.extension() != ".bms") {
                continue;
            }
            std::cout << "processing: " << f << "\n";
            try
            {
                convert_model(f);
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
