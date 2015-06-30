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

#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdint.h>
#include <string>

#include <common.h>
#include "model.h"

using namespace std;

void convert_model(string fn)
{
    buffer b(readFile(fn));
    model m;
    m.load(b);

    if (!b.eof())
    {
        stringstream ss;
        ss << hex << b.getIndex() << " != " << hex << b.getSize();
        throw std::logic_error(ss.str());
    }

    m.writeObj(fn);
}

int main(int argc, char *argv[])
try
{
    if (argc != 2)
    {
        printf("Usage: %s model_file \n", argv[0]);
        return 1;
    }
    convert_model(argv[1]);
    return 0;
}
catch (std::runtime_error &e)
{
    string error;
    if (argv[1])
        error += argv[1];
    error += "\n";
    error += "fatal error: ";
    error += e.what();
    error += "\n";
    if (argv[1])
    {
        ofstream ofile(string(argv[1]) + ".error.txt");
        ofile << error;
    }
    return 1;
}
catch (std::exception &e)
{
    printf("%s\n", argv[1]);
    printf("error: %s\n", e.what());
    return 1;
}
catch (...)
{
    printf("%s\n", argv[1]);
    printf("error: unknown exception\n");
    return 1;
}
