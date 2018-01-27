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

#include <primitives/filesystem.h>

using namespace std;

int main(int argc, char *argv[])
try
{
    if (argc < 2 || argc > 3)
    {
        cout << "Usage:\n" << argv[0] << " {file.mmp,mmp_dir} [texture_ids.txt]" << "\n";
        return 1;
    }

    auto func = [&argc, &argv](auto &p)
    {
        mmp m;
        if (argc > 2)
            m.loadTextureNames(argv[2]);
        m.load(p.string());
        m.process();
        m.writeFileInfo();
        //m.writeTexturesList();
        m.writeHeightMap();
        //m.writeHeightMapSegmented();
        /*m.writeTextureMap();
        m.writeTextureAlphaMaps();
        m.writeTextureMapColored();
        m.writeColorMap();*/
    };

    path p = argv[1];
    if (fs::is_regular_file(p))
        func(p);
    else if (fs::is_directory(p))
    {
        auto files = enumerate_files_like(p, ".*\\.mmp", false);
        for (auto &f : files)
        {
            std::cout << "processing: " << f << "\n";
            func(f);
        }
    }
    else
        throw std::runtime_error("Bad fs object");

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
