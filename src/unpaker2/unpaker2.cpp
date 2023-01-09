/*
 * AIM mod_converter
 * Copyright (C) 2015 lzwdgc
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

#include <buffer.h>

#include <primitives/filesystem.h>
#include <primitives/sw/main.h>
#include <primitives/sw/settings.h>
#include <primitives/sw/cl.h>
#include <primitives/templates2/mmap.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdint.h>
#include <string>

//#include <archive.h>
#include <lzma.h>
#include <lzo/lzo1x.h>

using namespace std;

constexpr auto supported_block_size = 32768;

#pragma pack(push, 1)
struct pak {
    uint32_t magic;
    uint16_t unk0;
    uint32_t n_files;
    uint32_t n_blocks;
    uint32_t block_size;
    uint32_t unk1;
};
struct file_description {
    const char name[0x50];
    uint32_t offset;
    uint32_t size;
};
struct segment {
    enum decode_algorithm : uint32_t {
        none = 0x0,
        lzo = 0x1,
        lzma = 0x2,
        rlew = 0x4,
    };

    uint32_t unk1; // some file offset? trash?
    decode_algorithm algorithm;
    uint32_t offset;
};
#pragma pack(pop)

struct stream {
    primitives::templates2::mmap_file<uint8_t> &m;
    uint8_t *p{m.p};

    template <typename T>
    operator T&() {
        auto &r = *(T*)p;
        p += sizeof(T);
        return r;
    }
    template <typename T>
    auto span(size_t len) {
        auto s = std::span<T>((T*)p, len);
        p += sizeof(T) * len;
        return s;
    }
};

struct decoded_block {
    uint8_t out[supported_block_size];
};

void unpack_file(path fn) {
    primitives::templates2::mmap_file<uint8_t> f{fn};
    stream s{f};
    pak p = s;
    if (p.block_size != supported_block_size) {
        throw std::runtime_error{"block size mismatch"};
    }
    auto descs = s.span<file_description>(p.n_files);
    auto segments = s.span<segment>(p.n_blocks);
    std::vector<uint8_t> bbb;
    bbb.resize(segments.size() * supported_block_size);
    auto pp = bbb.data();
    std::vector<decoded_block> dblocks;
    for (auto &&seg : segments) {
        s.p = f.p + seg.offset;
        auto &b = dblocks.emplace_back();
        uint32_t len = s;
        switch (seg.algorithm) {
        case segment::decode_algorithm::none: {
            //memcpy(b.out, s.p, len);
            memcpy(pp, s.p, len);
            break;
        }
        case segment::decode_algorithm::lzo: {
            size_t outsz = supported_block_size;
            //auto r2 = lzo1x_decompress(s.p, len, b.out, &outsz, 0);
            auto r2 = lzo1x_decompress(s.p, len, pp, &outsz, 0);
            if (r2 != LZO_E_OK) {
                throw std::runtime_error{"lzo error"};
            }
            break;
        }
        default:
            throw std::runtime_error{"compression unsupported"};
        }
        pp += len;
    }
    pp = bbb.data();

    /*uint8_t out[32768];
    uint64_t memlimit = 0;
    size_t in_pos = 0;
    size_t out_pos = 0;
    auto r = lzma_stream_buffer_decode(&memlimit, 0, 0, s.p, &in_pos, f.p+f.sz-s.p, out, &out_pos, 1'000'000'000);*/

    auto dir = fn += ".dir2";
    fs::create_directories(dir);
    for (auto &&d : descs) {
        auto fn = dir / d.name;
        fs::create_directories(fn.parent_path());
        std::ofstream o{fn, std::ios::binary};
        o.write((const char *)pp + d.offset, d.size);
    }
}

int main(int argc, char *argv[]) {
    cl::opt<path> p(cl::Positional, cl::desc("<pack file or dir>"), cl::Required);

    cl::ParseCommandLineOptions(argc, argv);

    if (fs::is_regular_file(p)) {
        unpack_file(p);
    } else if (fs::is_directory(p)) {
        auto files = enumerate_files_like(p, ".*\\.pak", false);
        for (auto &f : files) {
            if (f.has_extension())
                continue;
            std::cout << "processing: " << f << "\n";
            unpack_file(f);
        }
    } else {
        throw std::runtime_error("Bad fs object");
    }
    return 0;
}
