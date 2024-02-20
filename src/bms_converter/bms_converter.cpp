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
        uint32_t n_animations;
        uint32_t unk;
    };
    struct animation_header {
        uint32_t unk1;
        uint32_t n_unk1;
        uint32_t n_unk2;
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
    struct vertex {
        v coords;
        vn normal;
        vt tex;
    };
    struct f {
        uint16_t x, y;
    };
    struct animation_data {
        uint16_t vertex_id;
        v coords;
        vn normal;
    };
    using charC = char[0xC];
};
#pragma pack(pop)

void convert_model(const path &fn) {
    primitives::templates2::mmap_file<uint8_t> f{fn};
    stream s{f};
    bms::header h = s;
    auto names = s.span<bms::charC>(h.n_animations);
    uint32_t unk1 = s;
    auto animations = s.span<bms::animation_header>(h.n_animations);
    uint32_t n_vertices = s;
    uint32_t n_faces = s;
    auto vertices_read = s.span<bms::vertex>(n_vertices);
    std::vector<bms::vertex> vertices(vertices_read.begin(), vertices_read.end());
    auto vfs = s.span<uint16_t>(n_faces);

    auto print = [&vfs](const path &fn, auto &&vertices) {
        std::ofstream ofile{path{fn} += ".obj"};
        for (auto &&v : vertices) {
            ofile << std::format("v {} {} {}\n", v.coords.x, v.coords.y, v.coords.z);
        }
        ofile << "\n";
        for (auto &&v : vertices) {
            ofile << std::format("vt {} {}\n", v.tex.x, v.tex.y);
        }
        ofile << "\n";
        for (auto &&v : vertices) {
            ofile << std::format("vn {} {} {}\n", v.normal.x, v.normal.y, v.normal.z);
        }
        ofile << "\n";
        // ofile << "g obj\n";
        // ofile << "s 1\n";
        for (auto &&f : std::views::chunk(vfs, 3)) {
            // ofile << std::format("f {} {} {}\n", f[0] + 1, f[1] + 1, f[2] + 1);
            ofile << std::format("f {}/{}/{} {}/{}/{} {}/{}/{}\n", f[0] + 1, f[0] + 1, f[0] + 1, f[1] + 1, f[1] + 1,
                                 f[1] + 1, f[2] + 1, f[2] + 1, f[2] + 1);
        }
        // ofile << "g\n";
    };
    auto apply_animation = [&](auto &&anim) {
        //vertices.assign(vertices_read.begin(), vertices_read.end());
        for (auto &&a : anim) {
            memcpy(&vertices[a.vertex_id], &a.coords, sizeof(a) - sizeof(a.vertex_id));
        }
    };

    print(path{fn}, vertices);
    for (int id = 0; auto &&d : animations) {
        vertices.assign(vertices_read.begin(), vertices_read.end());

        auto anim1 = s.span<bms::animation_data>(d.n_unk1);
        apply_animation(anim1);
        print(path{fn} += std::format(".{}.1", names[id]), vertices);

        auto anim2 = s.span<bms::animation_data>(d.n_unk2);
        apply_animation(anim2);
        print(path{fn} += std::format(".{}.2", names[id]), vertices);
        ++id;
    }
}

int main(int argc, char *argv[])
{
    cl::opt<path> p(cl::Positional, cl::desc("<.bms file>"), cl::Required);

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
