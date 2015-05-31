/*
 * AIM mmp_extractor
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

#include "mmp.h"

using namespace std;

mmp read_mmp(string fn)
{
    mmp m;
    FILE *f = fopen(fn.c_str(), "rb");
    if (!f)
        return m;
    m.load(f);
    fclose(f);
    return m;
}

int main(int argc, char *argv[])
{
#ifdef NDEBUG
    if (argc != 2)
    {
        cout << "Usage:\n" << argv[0] << " file.mmp" << "\n";
        return 1;
    }
    read_mmp(argv[1]);
#else
    auto arena = read_mmp("h:\\Games\\Механоиды\\data\\maps.pak.dir\\arena.mmp");
    auto loc1 = read_mmp("h:\\Games\\Механоиды\\data\\maps.pak.dir\\location1.mmp");
#endif
    return 0;
}