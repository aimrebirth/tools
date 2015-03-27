/*
 * AIM obj_extractor
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
#include <stdio.h>
#include <string>

#include "objects.h"
#include "other.h"

void read_mmo(string fn)
{
    FILE *f = fopen(fn.c_str(), "rb");
    if (!f)
        return;
    Objects objects;
    objects.load(f);
    MechGroups mgs;
    mgs.load(f);
    if (feof(f))
    {
        // custom maps?
        fclose(f);
        return;
    }
    MapGoods mg;
    mg.load(f);
    MapMusic mm;
    mm.load(f);
    MapSounds ms;
    ms.load(f);
    fclose(f);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "Usage:\n" << argv[0] << " file.mmo" << "\n";
        return 1;
    }
    read_mmo(argv[1]);
    return 0;
}