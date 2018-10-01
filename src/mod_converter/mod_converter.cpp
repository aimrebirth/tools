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
#include "fbx.h"
#include "model.h"

#include <primitives/filesystem.h>
#include <primitives/sw/main.h>
#include <primitives/sw/settings.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdint.h>
#include <string>

using namespace std;

// options
bool silent = false;
bool printMaxPolygonBlock = false;
cl::opt<path> p(cl::Positional, cl::desc("<MOD_ file or directory with MOD_ files>"), cl::value_desc("file or directory"), cl::Required);

void convert_model(const path &fn)
{
    buffer b(read_file(fn));
    model m;
    m.load(b);

    if (!b.eof())
    {
        stringstream ss;
        ss << hex << b.index() << " != " << hex << b.size();
        throw std::logic_error(ss.str());
    }

    // write all
    if (all_formats)
        m.print(fn.string());
    m.printFbx(fn.string());
}

int main(int argc, char *argv[])
{
    cl::opt<bool> af("a", cl::desc("All formats"));
    cl::opt<bool> mr("mr", cl::desc("AIM Racing MOD file"));

    cl::ParseCommandLineOptions(argc, argv);

    if (mr)
        gameType = GameType::AimR;
    if (af)
        all_formats = true;

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
        throw std::runtime_error("Bad fs object");
    return 0;
}
