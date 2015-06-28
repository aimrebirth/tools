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
#include <iostream>
#include <map>
#include <set>
#include <stdio.h>
#include <stdint.h>
#include <string>

using namespace std;

#include "model.h"

model read_model(string fn)
{
    model m;
    FILE *f = fopen(fn.c_str(), "rb");
    if (!f)
        return m;
    try
    {
        m.load(f);
    }
    catch (std::exception &e)
    {
        printf("error: %s\n", fn.c_str());
        printf("%s\n", e.what());
        return m;
    }
    auto p = ftell(f);
    fseek(f, 0, SEEK_END);
    auto end = ftell(f);
    if (p != ftell(f))
    {
        printf("error: %s\n", fn.c_str());
        printf("     : %x != %x\n", p, end);
    }
    fclose(f);
    return m;
}

int main(int argc, char *argv[])
{
#ifdef NDEBUG
    if (argc != 2)
    {
        cout << "Usage:\n" << argv[0] << " model_file" << "\n";
        return 1;
    }
    read_model(argv[1]);
#else
#endif
    return 0;
}
