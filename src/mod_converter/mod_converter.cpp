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
#include "model.h"

#include <primitives/filesystem.h>
#include <primitives/sw/main.h>
#include <primitives/sw/settings.h>
#include <primitives/sw/cl.h>
#include <primitives/yaml.h>

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

cl::opt<path> p(cl::Positional, cl::desc("<MOD_ file or directory with MOD_ files>"), cl::value_desc("file or directory"), cl::Required);
cl::opt<bool> all_formats("af", cl::desc("All formats (.obj, .fbx)"));

yaml root;
cl::opt<bool> stats("i", cl::desc("Gather information from (models)"));

// https://twitter.com/FreyaHolmer/status/644881436982575104
// https://help.autodesk.com/view/FBX/2017/ENU/?guid=__cpp_ref_class_fbx_axis_system_html
cl::opt<AxisSystem> AS(cl::desc("Choose axis system (.fbx only):"),
    cl::values(
#define axisval(x, y) \
        cl::OptionEnumValue{ #x, (int)AxisSystem::x, y }

        axisval(eMayaZUp,         "UpVector = ZAxis, FrontVector = -ParityOdd, CoordSystem = RightHanded (         also 3dsMax, Blender)\n"
            "(Blender: when importing, disable 'Use Pre/Post Rotation')"),
        axisval(eMayaYUp,         "UpVector = YAxis, FrontVector =  ParityOdd, CoordSystem = RightHanded (default, also MotionBuilder, OpenGL)"),
        axisval(eDirectX,         "UpVector = YAxis, FrontVector =  ParityOdd, CoordSystem = LeftHanded  (         also Lightwave)")

#undef axisval
    )
    , cl::init(AxisSystem::Default)
);

auto read_model(const path &fn)
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

    return m;
}

void convert_model(const model &m, const path &fn)
{
    // write all
    if (all_formats)
        m.print(fn.u8string(), AS);
    m.printFbx(fn.u8string(), AS);
}

void convert_model(const path &fn)
{
    auto m = read_model(fn);

    if (stats)
    {
        m.save(root[fn.filename().u8string()]);
        return;
    }

    convert_model(m, fn);
}

int main(int argc, char *argv[])
{
    cl::opt<bool> mr("mr", cl::desc("AIM Racing MOD file"));

    cl::ParseCommandLineOptions(argc, argv);

    if (mr)
        gameType = GameType::AimR;

    if (fs::is_regular_file(p))
        convert_model(p);
    else if (fs::is_directory(p))
    {
        auto files = enumerate_files(p, false);
        for (auto &f : FilesSorted(files.begin(), files.end()))
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
        throw std::runtime_error("No such file or directory: " + normalize_path(p));

    if (stats)
    {
        write_file((fs::is_regular_file(p) ? path(p) += ".txt" : (p / "model_information.yml")) , YAML::Dump(root));
    }

    return 0;
}
