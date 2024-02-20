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
        uint32_t unk1; // type?
        uint32_t n_anim1;
        uint32_t n_anim2;
    };
    struct vec2 {
        float x, y;
        void operator+=(const vec2 &v) {
            x += v.x;
            y += v.y;
        }
    };
    struct vec3 : vec2 {
        float z;
        void operator+=(const vec3 &v) {
            vec2::operator+=(v);
            z += v.z;
        }
    };
    struct v : vec3 {};
    struct vn : vec3 {};
    struct vt : vec2 {};
    struct vertex {
        v coords;
        vn normal;
        vt tex;
    };
    using vertex_id = uint16_t;
    struct animation_data {
        vertex_id id;
        v coords;
        vn normal;
    };
    using charC = char[0xC];
    using animation_name = charC;

    static void convert_model(const path &fn) {
        primitives::templates2::mmap_file<uint8_t> f{fn};
        stream s{f};

        header h = s;
        auto animation_names = s.span<animation_name>(h.n_animations);
        uint32_t unk1 = s;
        auto animations = s.span<animation_header>(h.n_animations);
        uint32_t n_vertices = s;
        uint32_t n_faces = s;
        auto vertices_read = s.span<vertex>(n_vertices);
        auto faces = s.span<vertex_id>(n_faces);

        std::vector<vertex> vertices(vertices_read.begin(), vertices_read.end());
        auto print = [&faces](const path &fn, auto &&vertices) {
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
            for (auto &&f : std::views::chunk(faces, 3)) {
                ofile << std::format("f {}/{}/{} {}/{}/{} {}/{}/{}\n"
                    , f[0] + 1, f[0] + 1, f[0] + 1
                    , f[1] + 1, f[1] + 1, f[1] + 1
                    , f[2] + 1, f[2] + 1, f[2] + 1
                );
            }
            // ofile << "g\n";
        };
        auto apply_animation = [&](auto &&anim) {
            for (auto &&a : anim) {
                auto &v = vertices[a.id];
                v.coords += a.coords;
                v.normal += a.normal;
            }
        };

        print(path{fn}, vertices);
        for (int id = 0; auto &&d : animations) {
            vertices.assign(vertices_read.begin(), vertices_read.end());
            auto anim1 = s.span<animation_data>(d.n_anim1);
            apply_animation(anim1);
            print(path{fn} += std::format(".anim.{}.1", animation_names[id]), vertices);

            vertices.assign(vertices_read.begin(), vertices_read.end());
            auto anim2 = s.span<animation_data>(d.n_anim2);
            apply_animation(anim2);
            print(path{fn} += std::format(".anim.{}.2", animation_names[id]), vertices);

            vertices.assign(vertices_read.begin(), vertices_read.end());
            apply_animation(anim1);
            apply_animation(anim2);
            print(path{fn} += std::format(".anim.{}.merged", animation_names[id]), vertices);

            ++id;
        }
    }
};
#pragma pack(pop)

int main(int argc, char *argv[])
{
    cl::opt<path> p(cl::Positional, cl::desc("<.bms file or dir>"), cl::Required);

    cl::ParseCommandLineOptions(argc, argv);

    if (fs::is_regular_file(p)) {
        bms::convert_model(p);
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
                bms::convert_model(f);
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
