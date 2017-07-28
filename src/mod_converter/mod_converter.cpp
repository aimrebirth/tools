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

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdint.h>
#include <string>

#include <buffer.h>
#include "model.h"

using namespace std;

// options
bool all_formats = false;
bool silent = false;
bool printMaxPolygonBlock = false;
string filename;

bool parse_cmd(int argc, char *argv[]);

void convert_model(string fn)
{
    buffer b(readFile(fn));
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
        m.print(filename);
    m.printFbx(filename);
}

int main(int argc, char *argv[])
try
{
    if (argc < 2 || !parse_cmd(argc, argv))
    {
        printf("Usage: %s [OPTIONS] model_file\n", argv[0]);
        return 1;
    }
    convert_model(filename);
    return 0;
}
catch (std::runtime_error &e)
{
    if (silent)
        return 1;
    string error;
    error += filename;
    error += "\n";
    error += "fatal error: ";
    error += e.what();
    error += "\n";
    ofstream ofile(filename + ".error.txt");
    ofile << error;
    return 1;
}
catch (std::exception &e)
{
    if (silent)
        return 1;
    printf("%s\n", filename.c_str());
    printf("error: %s\n", e.what());
    return 1;
}
catch (...)
{
    if (silent)
        return 1;
    printf("%s\n", filename.c_str());
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
            filename = arg;
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
