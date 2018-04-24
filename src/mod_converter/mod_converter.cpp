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
#include <args.hxx>

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
path p;

bool parse_cmd(int argc, char *argv[]);

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
try
{
    args::ArgumentParser parser("mmo extractor");
    args::HelpFlag help(parser, "help", "Display this help menu", { 'h', "help" });
    args::Flag mr(parser, "mr", "AIM Racing MOD file", { "mr" });
    args::Positional<std::string> file_path(parser, "file or directory", "MOD_ file or directory with MOD_ files");
    parser.Prog(argv[0]);

    parser.ParseCLI(argc, argv);

    if (mr)
        gameType = GameType::AimR;

    p = file_path.Get();
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
/*catch (std::runtime_error &e)
{
    if (silent)
        return 1;
    string error;
    error += p.string();
    error += "\n";
    error += "fatal error: ";
    error += e.what();
    error += "\n";
    ofstream ofile(p.string() + ".error.txt");
    ofile << error;
    return 1;
}*/
catch (std::exception &e)
{
    if (silent)
        return 1;
    printf("%s\n", p.string().c_str());
    printf("error: %s\n", e.what());
    return 1;
}
catch (...)
{
    if (silent)
        return 1;
    printf("%s\n", p.string().c_str());
    printf("error: unknown exception\n");
    return 1;
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
