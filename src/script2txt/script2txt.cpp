/*
 * AIM script2txt
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

#include "script.h"

#include <script2txt_parser.h>

#include <primitives/filesystem.h>
#include <primitives/sw/main.h>
#include <primitives/sw/settings.h>
#include <primitives/sw/cl.h>

#include <fstream>
#include <iostream>
#include <stdint.h>

using std::cout;
using std::string;

int main(int argc, char *argv[])
{
    cl::opt<path> p(cl::Positional, cl::desc("<script.scr or scripts dir>"), cl::Required);

    cl::ParseCommandLineOptions(argc, argv);

    auto func = [](auto filename)
    {
        // read
        buffer b(read_file(filename));
        script s;
        s.load(b);
        auto str = s.get_text();

        Script2txtParserDriver driver;
        if (driver.parse(str))
        {
            throw std::runtime_error("error during parsing input file");
        }
        auto &ctx = driver.getContext();

        // write script
        {
            filename += ".txt";
            std::ofstream ofile(filename);
            if (ofile)
                ofile << ctx.getText();
        }

        // write function calls
        {
            std::ofstream functions("functions.txt", std::ios::app);
            if (functions)
            {
                for (auto &f : driver.functions)
                {
                    std::string f2(f.size(), 0);
                    std::transform(f.begin(), f.end(), f2.begin(), tolower);
                    functions << f2 << "\n";
                }
            }
        }
    };

    if (fs::is_regular_file(p))
        func(p.string());
    else if (fs::is_directory(p))
    {
        auto files = enumerate_files_like(p, ".*\\.scr", false);
        auto files2 = enumerate_files_like(p, ".*\\.QST", false);
        files.insert(files2.begin(), files2.end());
        for (auto &f : files)
        {
            std::cout << "processing: " << f << "\n";
            func(f.string());
        }
    }
    else
        throw std::runtime_error("Bad fs object");

    return 0;
}
