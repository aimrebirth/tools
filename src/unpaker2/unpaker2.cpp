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

#include <lzma.h>
#include <lzo/lzo1x.h>

using namespace std;

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

void unpack_file(path fn) {
    primitives::templates2::mmap_file<uint8_t> f{fn};
    stream s{f};
    pak p = s;
    auto descs = s.span<file_description>(p.n_files);
    auto segments = s.span<segment>(p.n_blocks);
    std::vector<uint8_t> bbb;
    bbb.resize((segments.size() + 1) * p.block_size * 4);
    auto pp = bbb.data();
    for (auto &&seg : segments) {
        s.p = f.p + seg.offset;
        uint32_t len = s;
        switch (seg.algorithm) {
        case segment::decode_algorithm::none: {
            memcpy(pp, s.p, len);
            pp += len;
            break;
        }
        case segment::decode_algorithm::lzo: {
            size_t outsz;
            auto r2 = lzo1x_decompress(s.p, len, pp, &outsz, 0);
            if (r2 != LZO_E_OK) {
                throw std::runtime_error{"lzo error"};
            }
            pp += outsz;
            break;
        }
        case segment::decode_algorithm::rlew: {
            auto base = s.p;
            uint16_t flag = s;
            while (s.p < base + len) {
                uint16_t w = s;
                if ((w & 0xFF00) == (flag << 8)) {
                    uint16_t count = (uint8_t)w;
                    if (count == 0xFF) {
                        uint16_t w2 = s;
                        *(decltype(w2) *)pp = w2;
                        pp += sizeof(w2);
                        continue;
                    }
                    uint16_t w2 = s;
                    count += 3;
                    while (count--) {
                        *(decltype(w2)*)pp = w2;
                        pp += sizeof(w2);
                    }
                } else {
                    *(decltype(w)*)pp = w;
                    pp += sizeof(w);
                }
            }
            break;
        }
        case segment::decode_algorithm::lzma: {
            uint8_t flags = s;

            lzma_stream strm{};
            strm.next_in = s.p;
            strm.avail_in = len;
            strm.next_out = pp;
            strm.avail_out = p.block_size;

            auto r = lzma_lzip_decoder(&strm, 10'000'000, flags);
            if (r != LZMA_OK) {
                throw std::runtime_error{"lzma error"};
            }
            r = lzma_code(&strm, LZMA_RUN);
            if (r != LZMA_STREAM_END) {
                throw std::runtime_error{"lzma error"};
            }
            pp += strm.total_out;
            break;
        }
        default:
            throw std::runtime_error{"compression unsupported: "s + std::to_string(seg.algorithm)};
        }
    }
    pp = bbb.data();

    auto dir = fn += ".dir2";
    fs::create_directories(dir);
    for (auto &&d : descs) {
        auto fn = dir / d.name;
        fs::create_directories(fn.parent_path());
        std::cout << "unpacking " << fn << "\n";
        primitives::templates2::mmap_file<uint8_t> f{fn, primitives::templates2::mmap_file<uint8_t>::rw{}};
        f.alloc_raw(d.size);
        memcpy(f.p, pp + d.offset, d.size);
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
            std::cout << "processing: " << f << "\n";
            try {
                unpack_file(f);
            } catch (std::exception &e) {
                std::cerr << e.what() << "\n";
            }
        }
    } else {
        throw std::runtime_error("Bad fs object");
    }
    return 0;
}
