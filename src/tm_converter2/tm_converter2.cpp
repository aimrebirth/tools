/*
 * AIM tm_converter
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

#include <color.h>
#include <mmap.h>

#include <primitives/filesystem.h>
#include <primitives/sw/main.h>
#include <primitives/sw/settings.h>
#include <primitives/sw/cl.h>
#include <s3tc.h>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <bit>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdint.h>
#include <string>

#ifdef _WIN32
#include <d3d9.h>
#endif

/*
aim icons
TEX_KT_ICONS.TM
64 x 32
16 icon in a column
*/

// TODO: add dxt5 compressor from stb libs (stb_dxt5)
// see org.sw.demo.stb.all

using namespace std;

#pragma pack(push, 1)
struct tm_file {
    int32_t width;
    int32_t height;
    // https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dformat
#ifdef _WIN32
    D3DFORMAT d3dformat;
#else
    uint32_t d3dformat;
#endif
    // see for some infos https://learn.microsoft.com/en-us/previous-versions/ms889290(v=msdn.10)
    // IDirect3DDevice8::CreateTexture
    // or not? or number of helper images or total imgs
    uint32_t levels;
    // or not only dxt5? maybe enum?
    uint32_t dxt5_compression;
    // TEX_NIGHT13.TM has 1 here
    // TEX_kt_icons.TM has 8 here
    // maybe levels?
    // number of anims? icon variants?
    uint32_t unk0;
    uint8_t unkX[0x4C-0x18];
};
#pragma pack(pop)

void convert(const path &fn)
{
    primitives::templates2::mmap_file<uint8_t> f{fn};
    stream s{f};
    tm_file tm = s;

    // format check
    switch (tm.d3dformat) {
    case D3DFMT_A8R8G8B8:
        break;
    case D3DFMT_X8R8G8B8:
        break;
    default:
        SW_UNIMPLEMENTED;
    }

    // compression check
    switch (tm.dxt5_compression) {
    case 0:
        break;
    case 1:
        break;
    default:
        SW_UNIMPLEMENTED;
    }

    auto save = [&](auto &&sub) {
        cv::Mat m(tm.width, tm.height, CV_8UC4);
        if (tm.dxt5_compression) {
            // this have some one off results
            // probaby we did different rounding in current (v1) impl
            BlockDecompressImageDXT5(tm.width, tm.height, s.p, (unsigned long *)m.data);
            s.skip(tm.width * tm.height);
            // only for dxt5
            m.forEach<cv::Vec4b>([](auto &v, auto *pos) {
                // ARGB -> BGRA (big endian)
                // but in bytes (little endian)
                // 0xBGRA -> 0xABGR
                auto &val = *(uint32_t *)&v;
                val = std::rotr(val, 8);
            });
            if (tm.d3dformat == D3DFMT_X8R8G8B8) {
                // and shrink if possible
                cv::cvtColor(m, m, cv::COLOR_BGRA2BGR);
            }
        }
        else
        {
            // 'simple' files consists of:
            // 1. 'normal' image (bitmap)
            // 2. some kind of mask?
            // 3. rest? levels?
            // but every tm has multiple levels or so

            int size = tm.width * tm.height * 2;
            auto dst = m.data;
            for (int i = 0; i < size; ++i) {
                uint8_t c = s;
                uint8_t lo = c & 0x0F;
                uint8_t hi = (c & 0xF0) >> 4;
                *dst++ = (lo << 4) | lo;
                *dst++ = (hi << 4) | hi;
            }
        }
        // opencv can't save to tga directly
        cv::imwrite((path(fn) += sub + ".bmp"s).string(), m);
    };
    save("");
    // not sure how to parse rest, probably dx8 generated texture levels
    // and we dont need them
    return;

    if (!tm.dxt5_compression) {
        save("_mask"); // what is it?
    }
    for (int i = 0; i < tm.unk0; ++i) {
        // obviosly small
        tm.width /= 2;
        tm.height /= 2;
        save(std::format("_small{}", i + 1));
        if (!tm.dxt5_compression) {
            save(std::format("_small{}_mask", i + 1));
        }
    }

    if (s.p - f.p != f.sz) {
        int a = 5;
        a++;
        SW_UNIMPLEMENTED;
    }
}

int main(int argc, char *argv[])
{
    cl::list<path> list(cl::Positional, cl::desc("<files.tm or dirs>"), cl::Required, cl::OneOrMore);

    cl::ParseCommandLineOptions(argc, argv);

    for (auto &&p : list) {
        if (fs::is_regular_file(p)) {
            convert(p);
        } else if (fs::is_directory(p)) {
            auto files = enumerate_files_like(p, ".*\\.TM", false);
            for (auto &f : files) {
                std::cout << "processing: " << to_printable_string(f) << "\n";
                convert(f);
            }
        } else {
            throw std::runtime_error("Bad fs object");
        }
    }
    return 0;
}
