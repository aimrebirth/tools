/*
 * AIM 1 unpaker
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

#include "pak.h"

void unpak(string fn)
{
    FILE *f = fopen(fn.c_str(), "rb");
    if (!f)
        return;
    pak p;
    p.load(f);

    auto unpack = [&](auto &file)
    {
        cout << "Unpacking " << file.name << "\n";
        vector<char> buf(file.len);
        file.read(&p, &buf[0], file.len);
        file.write(fn + ".dir", buf);
    };

    for (auto &[n,f] : p.files)
        unpack(f);
    fclose(f);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " archive.pak" << "\n";
        return 1;
    }
    unpak(argv[1]);
    return 0;
}
