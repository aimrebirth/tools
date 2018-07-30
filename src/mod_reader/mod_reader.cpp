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
#include <model.h>

#include <primitives/filesystem.h>
#include <primitives/sw/main.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdint.h>
#include <string>

using namespace std;

// options
bool all_formats = false;
bool silent = false;
bool printMaxPolygonBlock = false;
path p;

bool parse_cmd(int argc, char *argv[]);

void convert_model(const path &fn)
{
    buffer b(read_file(fn));
    block bl;
    bl.loadPayload(b);

    if (!b.eof())
    {
        stringstream ss;
        ss << hex << b.index() << " != " << hex << b.size();
        throw std::logic_error(ss.str());
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2 || !parse_cmd(argc, argv))
    {
        printf("Usage: %s [OPTIONS] {model_file,model_dir}\n", argv[0]);
        return 1;
    }
    if (fs::is_regular_file(p))
        convert_model(p);
    else if (fs::is_directory(p))
    {
        auto files = enumerate_files(p, false);
        for (auto &f : files)
        {
            if (f.has_extension())
                continue;
            std::cout << "processing: " << f << "\n";
            convert_model(f);
        }
    }
    else
        throw std::runtime_error("Bad fs object");
    return 0;
}

bool parse_cmd(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        auto arg = argv[i];
        if (*arg != '-')
        {
            if (i != argc - 1)
                return false;
            p = arg;
            continue;
        }
        switch (arg[1])
        {
        case 'a':
            all_formats = true;
            break;
        case 's':
            silent = true;
            break;
        case 'm':
            printMaxPolygonBlock = true;
            break;
        }
    }
    return true;
}
