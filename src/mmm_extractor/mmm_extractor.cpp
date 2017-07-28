/*
 * AIM mmm_extractor
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

#include <iostream>
#include <stdint.h>
#include <string>
#include <sstream>

#include <buffer.h>
#include <dxt5.h>

#include <primitives/filesystem.h>

using namespace std;

struct mmm
{
    uint32_t unk1;
    uint32_t unk2;
    dxt5 data;

    void load(const buffer &b)
    {
        READ(b, unk1);
        READ(b, unk2);
        data.load(b);
    }
};

mmm read_mmm(const path &fn)
{
    buffer b(read_file(fn, true));
    mmm m;
    m.load(b);

    if (!b.eof())
    {
        std::stringstream ss;
        ss << std::hex << b.index() << " != " << hex << b.size();
        throw std::logic_error(ss.str());
    }
    return m;
}

void process_mmm(const path &fn)
{
    auto m = read_mmm(fn);
    write_mat_bmp(fn.string() + ".bmp", m.data.unpack_mmm());
}

int main(int argc, char *argv[])
try
{
    if (argc != 2)
    {
        cout << "Usage:\n" << argv[0] << " {file.mmm,dir}" << "\n";
        return 1;
    }
    path p = argv[1];
    if (fs::is_regular_file(p))
        process_mmm(p);
    else if (fs::is_directory(p))
    {
        auto files = enumerate_files_like(p, ".*\\.mmm", false);
        for (auto &f : files)
        {
            std::cout << "processing: " << f << "\n";
            process_mmm(f);
        }
    }
    else
        throw std::runtime_error("Bad fs object");
    return 0;
}
catch (std::exception &e)
{
    printf("error: %s\n", e.what());
    return 1;
}
catch (...)
{
    printf("error: unknown exception\n");
    return 1;
}
