/*
 * AIM unpaker (for AIM and AIM2 games, AIM:R)
 * Copyright (C) 2023 lzwdgc
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

#include <common.h>
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

#include <lzma.h>
#include <lzo/lzo1x.h>

#include "decode.h"

using namespace std;

void unpack_file(path fn) {
    primitives::templates2::mmap_file<uint8_t> f{fn};
    stream s{f};
    pak p = s;
    auto descs = s.span<pak::file_description>(p.n_files);
    auto segments = s.span<pak::segment>(p.n_blocks);
    std::vector<uint8_t> decoded;
    decoded.resize((segments.size() + 1) * p.block_size * 4);
    auto pp = decoded.data();
    progress_bar pb{segments.size(), 50};
    for (auto &&seg : segments) {
        s.p = f.p + seg.offset;
        uint32_t len = s;
        auto m2 = [&]() {
            enum decode_algorithm : uint32_t {
                none = 0x0,
                lzo = 0x1,
                lzma = 0x2,
                rlew = 0x4, // https://moddingwiki.shikadi.net/wiki/Id_Software_RLEW_compression
            };
            switch (seg.algorithm) {
            case decode_algorithm::none: {
                memcpy(pp, s.p, len);
                pp += len;
                break;
            }
            case decode_algorithm::lzo: {
                size_t outsz;
                // use lzo1x_decompress_safe?
                auto r2 = lzo1x_decompress(s.p, len, pp, &outsz, 0);
                if (r2 != LZO_E_OK) {
                    throw std::runtime_error{"lzo error"};
                }
                pp += outsz;
                break;
            }
            case decode_algorithm::rlew: {
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
            case decode_algorithm::lzma: {
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
        };
        auto m1 = [&]() {
            enum decode_algorithm : uint32_t {
                None = 0x0,
                RLE_2_bytes = 0x1,
                RLE_1_byte = 0x2,
                decode_algorithm_1 = 0x4, // not used
                decode_algorithm_2 = 0x8,
            };
            auto in = s.p;
            auto size1 = len;
            std::vector<uint8_t> vec;
            if (seg.algorithm & decode_algorithm_1) {
                // if you see this, check in git history decode_f1()
                throw std::runtime_error{"compression unsupported: "s + std::to_string(seg.algorithm)};
            }
            if (seg.algorithm & decode_algorithm_2) {
                uint32_t size2 = s;
                vec.resize(std::max(size2 * 4, p.block_size));
                decode_f2((char *)s.p, size2, (char *)vec.data());
                in = vec.data();
            }
            if (seg.algorithm & RLE_2_bytes) {
                pp = decode_rle((uint16_t *)in, size1, (uint16_t *)pp);
            } else if (seg.algorithm & RLE_1_byte) {
                pp = decode_rle((uint8_t *)in, size1, (uint8_t *)pp);
            }
            if (seg.algorithm == None) {
                memcpy(pp, s.p, size1);
            }
        };
        if (p.magic == 0) {
            m1();
        } else {
            m2();
        }
        pb.step();
    }
    std::cout << "\n";
    auto dir = fn += ".dir";
    fs::create_directories(dir);
    for (auto &&d : descs) {
        auto fn = dir / d.name;
        fs::create_directories(fn.parent_path());
        std::cout << "unpacking " << fn << "\n";
        primitives::templates2::mmap_file<uint8_t> f{fn, primitives::templates2::mmap_file<uint8_t>::rw{}};
        f.alloc_raw(d.size);
        memcpy(f.p, decoded.data() + d.offset, d.size);
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
