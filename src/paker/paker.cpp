/*
 * AIM paker (for AIM1 game only)
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

#include <primitives/filesystem.h>
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

using namespace std;

int main(int argc, char *argv[]) {
    cl::opt<path> name(cl::Positional, cl::desc("<pack name>"), cl::Required);
    cl::list<String> in_files(cl::Positional, cl::desc("<files to pack>"), cl::Required, cl::OneOrMore);

    cl::ParseCommandLineOptions(argc, argv);

    struct file {
        path fn;
        String alias;
        auto operator<=>(const file &) const = default;
    };
    std::set<file> files;
    for (auto &&f : in_files) {
        auto s = f;
        boost::replace_all(s, "/", "\\");
        boost::to_lower(s);
        auto v = split_string(s, "=");
        if (v.size() > 2 || v.empty()) {
            throw std::runtime_error("bad input filename: "s + f);
        }
        if (v.size() == 1) {
            path p{s};
            String alias;
            // some heuristics
            if (p.extension() == ".qst" || p.extension() == ".scr") {
                alias = "script\\bin\\" + s;
            }
            files.emplace(v[0], alias);
        } else if (v.size() == 2) {
            files.emplace(v[0], v[1]);
        }
    }

    size_t total{};
    for (auto &[f,_] : files) {
        total += fs::file_size(f);
    }
    uint32_t block_size = pak::default_block_size;
    uint32_t block_size_len = sizeof(block_size);
    block_size = total + block_size_len;
    uint32_t block_size_minus_len = block_size - block_size_len; // minus block len
    auto get_nsegs = [&](auto sz) {
        auto nsegs = sz / block_size_minus_len;
        if (nsegs == 0 || (sz % block_size_minus_len) != 0) {
            ++nsegs;
        }
        return nsegs;
    };
    auto nsegs = get_nsegs(total);
    total = 0;
    total += files.size() * sizeof(pak::file_description);
    total += nsegs * (sizeof(pak::segment) + block_size);
    total += sizeof(pak);

    pak p{};
    p.block_size = block_size;
    p.n_files = files.size();
    p.n_blocks = nsegs;

    primitives::templates2::mmap_file<uint8_t> f{name, primitives::templates2::mmap_file<uint8_t>::rw{}};
    f.alloc_raw(total);

    stream s{f};
    s = p;
    size_t offset = 0;
    for (auto &[name,alias] : files) {
        pak::file_description d;
        auto str = alias.empty() ? name.string() : alias;
        strcpy(d.name, str.c_str());
        d.size = fs::file_size(name);
        d.offset = offset;
        offset += d.size;
        s = d;
    }
    for (int i = 0; i < nsegs; ++i) {
        pak::segment seg{};
        seg.algorithm = 0; // NOTE: no compression!
        seg.offset = sizeof(pak) + files.size() * sizeof(pak::file_description) + nsegs * sizeof(pak::segment) + i * block_size;
        s = seg;
    }
    //uint32_t current_seg_len = 0;
    s = block_size_minus_len; // for single seg
    for (auto &[name,_] : files) {
        std::cout << "processing: " << name << "\n";
        primitives::templates2::mmap_file<uint8_t> f{name};
        auto sz = f.sz;
        uint32_t sz_to_copy = sz;//sz > block_size_minus_len ? block_size_minus_len : sz;
        //s = block_size_minus_len;
        memcpy(s.p, f.p, sz_to_copy);
        s.skip(sz_to_copy);
        //current_seg_len += block_size_len + sz_to_copy;
    }
    f.close();
    fs::resize_file(name, total);

    return 0;
}
