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

#include <iostream>
#include <set>
#include <stdint.h>
#include <string>
#include <sstream>

#include "mmp.h"

using namespace std;

int main(int argc, char *argv[])
try
{
    if (argc < 2 || argc > 3)
    {
        cout << "Usage:\n" << argv[0] << " file.mmp [texture_ids.txt]" << "\n";
        return 1;
    }
    mmp m;
    if (argc > 2)
        m.loadTextureNames(argv[2]);
    m.load(argv[1]);
    m.process();
    m.writeFileInfo();
    m.writeTexturesList();
    m.writeHeightMap();
    m.writeTextureMap();
    m.writeTextureAlphaMaps();
    m.writeTextureMapColored();
    m.writeColorMap();
    return 0;
}
catch (std::exception &e)
{
    printf("error: %s\n", e.what());
    return 1;
}
catch (...)
{
    printf("error: unknown exception\n");
    return 1;
}
