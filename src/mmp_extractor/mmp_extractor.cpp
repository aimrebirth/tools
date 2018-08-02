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

#include <opencv2/highgui.hpp>
#include "mmp.h"

#include <primitives/filesystem.h>
#include <primitives/sw/main.h>
#include <primitives/sw/settings.h>

#include <iostream>
#include <set>
#include <stdint.h>
#include <string>
#include <sstream>

using namespace std;

int main(int argc, char *argv[])
{
    cl::opt<path> p(cl::Positional, cl::desc("<file.mmp or directory>"), cl::Required);
    cl::opt<path> texture_ids(cl::Positional, cl::desc("<path to texture_ids.txt>"));
    cl::opt<bool> split_colormap("split_colormap", cl::desc("split colormap into separate images"));

    cl::ParseCommandLineOptions(argc, argv);

    auto func = [&texture_ids, &split_colormap](auto &p)
    {
        mmp m;
        if (!texture_ids.empty())
            m.loadTextureNames(texture_ids);
        m.load(p);
        m.process();
        m.writeFileInfo();
        m.writeTexturesList();
        m.writeHeightMap();
        //m.writeHeightMapSegmented();
        m.writeTextureMap();
        m.writeTextureAlphaMaps();
        m.writeTextureMapColored();
        m.writeColorMap();
        m.writeShadowMap();
        m.writeNormalMap();
        if (split_colormap)
            m.writeSplitColormap();
    };

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
