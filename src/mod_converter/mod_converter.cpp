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

#include <sstream>
#include <stdio.h>
#include <stdint.h>
#include <string>

#include <common.h>
#include "model.h"

using namespace std;

void convert_model(string fn)
{
    printf("%s\n", fn.c_str());

    buffer b(readFile(fn));
    model m;
    m.load(b);

    if (!b.eof())
    {
        stringstream ss;
        ss << hex << b.getIndex() << " != " << hex << b.getSize();
        throw std::logic_error(ss.str());
    }

    m.writeObj(fn + ".obj");
}

int main(int argc, char *argv[])
try
{
#ifdef NDEBUG
    if (argc != 2)
    {
        cout << "Usage:\n" << argv[0] << " model_file" << "\n";
        return 1;
    }
    read_model(argv[1]);
#else
    convert_model("h:\\Games\\AIM\\data\\res0.pak.dir\\Data\\Models\\MOD_GL_M1_A_ATTACKER");
    convert_model("h:\\Games\\AIM\\data\\res0.pak.dir\\Data\\Models\\MOD_GL_M1_B_BASE");

    convert_model("h:\\Games\\AIM\\data\\res0.pak.dir\\Data\\Models\\MOD_BLD_BASE1");
    convert_model("h:\\Games\\AIM\\data\\res0.pak.dir\\Data\\Models\\MOD_GL_S4_SINIGR");
    
    convert_model("h:\\Games\\AIM\\data\\res0.pak.dir\\Data\\Models\\MOD_GL_FIRE");
    convert_model("h:\\Games\\AIM\\data\\res0.pak.dir\\Data\\Models\\MOD_GL_FARA");

    convert_model("h:\\Games\\AIM\\data\\res0.pak.dir\\Data\\Models\\MOD_FX_ANTI_MATER_GUN");
    convert_model("h:\\Games\\AIM\\data\\res0.pak.dir\\Data\\Models\\MOD_UNFL_STONE01");
    convert_model("h:\\Games\\AIM\\data\\res0.pak.dir\\Data\\Models\\MOD_L1_KUST");
    convert_model("h:\\Games\\AIM\\data\\res0.pak.dir\\Data\\Models\\MOD_L6_KUST_12");
    convert_model("h:\\Games\\AIM\\data\\res0.pak.dir\\Data\\Models\\MOD_GL_M1_A_ATTACKER_DAMAGED");
#endif
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