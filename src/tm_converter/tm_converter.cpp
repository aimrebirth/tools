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

#include <bmp.h>
#include <buffer.h>
#include <dxt5.h>

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

void convert_simple(buffer &dst, const buffer &src, int width, int height)
{
    int size = width * height * 2;
    for (int i = 0; i < size; i++)
    {
        uint8_t c;
        READ(src, c);
        uint8_t lo = c & 0x0F;
        uint8_t hi = (c & 0xF0) >> 4;
        dst.write(uint8_t((lo << 4) | lo));
        dst.write(uint8_t((hi << 4) | hi));
    }
}

void convert(const path &fn)
{
    int width, height;
    int dxt5_flag = 0;

    buffer src(read_file(fn));
    READ(src, width);
    READ(src, height);
    src.seek(0x10);
    src._read(&dxt5_flag, 1);
    src.seek(0x4C);

    auto s = path(fn) += ".bmp";
    mat<uint32_t> m(width, height);
    if (dxt5_flag)
    {
        dxt5 d;
        d.width = width;
        d.height = height;
        d.load_blocks(src);
        m = d.unpack_tm();
    }
    else
    {
        buffer dst2;
        convert_simple(dst2, src, width, height);
        dst2.reset();
        memcpy(&m(0,0), dst2.getPtr(), dst2.size());
        m = m.flip(); // flip tga (normal rows order) to bmp (inverse rows order)
    }
    write_mat_bmp(s, m);
}

int main(int argc, char *argv[])
{
    cl::list<path> list(cl::Positional, cl::desc("<files.tm or dirs>"), cl::Required, cl::OneOrMore);

    cl::ParseCommandLineOptions(argc, argv);

    for (auto &&p : list)
    if (fs::is_regular_file(p))
        convert(p);
    else if (fs::is_directory(p))
    {
        auto files = enumerate_files_like(p, ".*\\.TM", false);
        for (auto &f : files)
        {
            std::cout << "processing: " << to_printable_string(f) << "\n";
            convert(f);
        }
    }
    else
        throw std::runtime_error("Bad fs object");
    return 0;
}
