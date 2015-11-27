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
#include <sstream>
#include <stdio.h>
#include <stdint.h>
#include <string>

#include <buffer.h>
#include "model.h"

using namespace std;

// options
bool silent = false;
bool printMaxPolygonBlock = false;
string filename;

bool parse_cmd(int argc, char *argv[]);

void print(const block &b, const std::string &fn)
{
    if (b.type == BlockType::ParticleEmitter)
        return;
    
    auto obj_fn = fn;
    if (!printMaxPolygonBlock)
        obj_fn += string(".") + b.name;
    obj_fn += ".obj";
    ofstream o(obj_fn);
    o << "#" << "\n";
    o << "# A.I.M. Model Converter (ver. " << version() << ")\n";
    o << "#" << "\n";
    o << "\n";
    int p1 = fn.rfind("\\");
    int p2 = fn.rfind("/");
    auto mtl = fn.substr(std::max(p1, p2) + 1);
    if (!printMaxPolygonBlock)
        mtl += string(".") + b.name;
    o << "mtllib " << mtl << ".mtl\n";
    o << "\n";
    o << b.printObj(mtl);

    auto mtl_fn = fn;
    if (!printMaxPolygonBlock)
        mtl_fn += string(".") + b.name;
    mtl_fn += ".mtl";
    ofstream m(mtl_fn);
    m << "#" << "\n";
    m << "# A.I.M. Model Converter (ver. " << version() << ")\n";
    m << "#" << "\n";
    m << "\n";
    m << b.printMtl(mtl);
}

void convert_model(string fn)
{
    buffer b(readFile(fn));
    model m;
    m.load(b);

    if (!b.eof())
    {
        stringstream ss;
        ss << hex << b.index() << " != " << hex << b.size();
        throw std::logic_error(ss.str());
    }

    // write obj and mtl
    if (printMaxPolygonBlock)
    {
        int max = 0;
        int maxBlock = -1;
        for (int i = 0; i < m.blocks.size(); i++)
        {
            if (m.blocks[i].n_vertex > max)
            {
                max = m.blocks[i].n_vertex;
                maxBlock = i;
            }
        }
        print(m.blocks[maxBlock], filename);
    }
    else
    {
        for (auto &f : m.blocks)
            print(f, filename);
    }
}

int main(int argc, char *argv[])
try
{
    if (argc < 2 || !parse_cmd(argc, argv))
    {
        printf("Usage: %s [OPTIONS] model_file\n", argv[0]);
        return 1;
    }
    convert_model(filename);
    return 0;
}
catch (std::runtime_error &e)
{
    if (silent)
        return 1;
    string error;
    error += filename;
    error += "\n";
    error += "fatal error: ";
    error += e.what();
    error += "\n";
    ofstream ofile(filename + ".error.txt");
    ofile << error;
    return 1;
}
catch (std::exception &e)
{
    if (silent)
        return 1;
    printf("%s\n", filename.c_str());
    printf("error: %s\n", e.what());
    return 1;
}
catch (...)
{
    if (silent)
        return 1;
    printf("%s\n", filename.c_str());
    printf("error: unknown exception\n");
    return 1;
}

bool parse_cmd(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        auto arg = argv[i];
        if (*arg != '-')
        {
            if (i != argc - 1)
                return false;
            filename = arg;
            continue;
        }
        switch (arg[1])
        {
        case 's':
            silent = true;
            break;
        case 'm':
            printMaxPolygonBlock = true;
            break;
        }
    }
    return true;
}
