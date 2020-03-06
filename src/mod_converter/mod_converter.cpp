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

/*
TODO:
    - ue4 .obj import not working (model looks ugly)
    - .fbx export with link faces not working
*/

// options
bool silent = false;
bool printMaxPolygonBlock = false;

cl::opt<path> p(cl::Positional, cl::desc("<MOD_ file or directory with MOD_ files or .mod file saved from AIM2 SDK viewer>"), cl::value_desc("file or directory"), cl::Required);
cl::opt<bool> all_formats("af", cl::desc("All formats (.obj, .fbx)"));
// link_faces is not currently complete, after processing we have bad uvs
cl::opt<bool> link_faces("lf", cl::desc("Link faces (default: true)")/*, cl::init(true)*/);

yaml root;
cl::opt<bool> stats("i", cl::desc("Gather information from (models)"));

// https://twitter.com/FreyaHolmer/status/644881436982575104
// https://help.autodesk.com/view/FBX/2017/ENU/?guid=__cpp_ref_class_fbx_axis_system_html
cl::opt<AxisSystem> AS(cl::desc("Choose axis system (.fbx only):"),
    cl::values(
#define axisval(x, y) \
        cl::OptionEnumValue{ #x, (int)AxisSystem::x, y }
#define axisalias(x, a) \
    cl::OptionEnumValue{ #x, (int)AxisSystem::a, "Same as " #a }

        axisval(eMayaZUp,         "UpVector = ZAxis, FrontVector = -ParityOdd, CoordSystem = RightHanded (         also 3dsMax, Blender)\n"
            "(Blender: when importing, disable 'Use Pre/Post Rotation')"),
        axisalias(eMax, eMayaZUp),
        axisalias(eBlender, eMayaZUp),

        axisval(eMayaYUp,         "UpVector = YAxis, FrontVector =  ParityOdd, CoordSystem = RightHanded (default, also MotionBuilder, OpenGL)"),
        axisalias(eMotionbuilder, eMayaYUp),
        axisalias(eOpenGL, eMayaYUp),

        axisval(eDirectX,         "UpVector = YAxis, FrontVector =  ParityOdd, CoordSystem = LeftHanded  (         also Lightwave)"),
        axisalias(eLightwave, eDirectX),

        axisval(eWindows3DViewer, "UpVector = ZAxis, FrontVector =  ParityOdd, CoordSystem = RightHanded (         also Lightwave)")

#undef axisval
#undef axisalias
    )
    , cl::init(AxisSystem::Default)
);

auto read_model(const path &fn)
{
    buffer b(read_file(fn));
    model m;
    if (fn.extension() == ".mod") // single block file from m2 sdk viewer
    {
        block bl;
        bl.h.name = fn.stem().u8string();
        bl.loadPayload(b);
        m.blocks.push_back(bl);
    }
    else
    {
        m.load(b);
    }
    if (link_faces)
        m.linkFaces();

    if (!b.eof())
    {
        std::stringstream ss;
        ss << std::hex << b.index() << " != " << std::hex << b.size();
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
