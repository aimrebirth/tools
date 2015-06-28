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

#include "model.h"

#include <fstream>
#include <string>
#include <time.h>

using namespace std;

std::string version();

#define FREAD(var) fread(&var, 1, sizeof(var), f)
#define SREAD(var) s.read(&var, sizeof(var))
#define SREAD_N(var, sz) s.read(&var, sz)

void vertex::load(s_file &s, uint32_t flags)
{
    SREAD(vX);
    SREAD(vZ);
    SREAD(vY);

    if (flags & F_WIND)
        SREAD(wind);
    
    SREAD(nX);
    SREAD(nZ);
    SREAD(nY);
    
    SREAD(t1);
    SREAD(t2);
}

std::string vertex::printVertex() const
{
    string s;
    s = "v  " + to_string(-vX) + " " + to_string(vY) + " " + to_string(-vZ);
    return s;
}

std::string vertex::printNormal() const
{
    string s;
    s = "vn " + to_string(-nX) + " " + to_string(nY) + " " + to_string(-nZ);
    return s;
}

std::string vertex::printTex() const
{
    string s;
    s = "vt " + to_string(t1) + " " + to_string(1 - t2) + " " + to_string(0);
    return s;
}

void fragment::load(FILE *f)
{
    FREAD(type);
    FREAD(name0);
    FREAD(name1);
    FREAD(name2);
    FREAD(name3);
    FREAD(name4);
    FREAD(unk0);
    FREAD(unk1);
    FREAD(unk2);
    FREAD(unk3);
    FREAD(size);
    FREAD(unk4);
    data.resize(size);
    data_offset = ftell(f);
    fread(data.data(), 1, size, f);
}

bool fragment::extract()
{
    s_file s(data, data_offset);
    
    SREAD(n_segments);
    segments.resize(n_segments);
    SREAD(header);
    SREAD(triangles_mult_7);
    SREAD(unk10);
    SREAD(flags);
    SREAD(n_vertex);
    vertices.resize(n_vertex);
    SREAD(n_triangles);
    if (triangles_mult_7)
        n_triangles *= 7;
    triangles.resize(n_triangles);
    for (auto &v : vertices)
        v.load(s, flags);
    for (auto &t : triangles)
        SREAD(t);

    for (auto &seg : segments)
    {
        uint32_t type;
        SREAD(type);
        switch (type)
        {
        case 1:
            seg = new segment1;
            break;
        case 2:
            seg = new segment2;
            break;
        case 6:
            seg = new segment6;
            break;
        default:
            throw std::logic_error("unknown segment type " + std::to_string(type));
        }
        seg->type = type;
        seg->extract(s);
    }

    return s.eof();
}

void segment1::extract(s_file &s)
{
    SREAD(name);
    SREAD(unk0);
    triangles.resize(unk0[0][0]);
    unk1.resize(unk0[0][0]);
    for (int i = 0; i < 2; i++)
    {
        for (auto &t : triangles)
            SREAD(t);
        for (auto &unk: unk1)
            SREAD(unk);
    }
}

void segment2::extract(s_file &s)
{
    SREAD(name);
    SREAD(unk0);
    triangles.resize(unk0[0][0]);
    unk1.resize(unk0[0][0]);
    unk1_1.resize(unk0[0][0]);
    for (auto &t : triangles)
        SREAD(t);
    for (auto &unk : unk1)
        SREAD(unk);
    for (auto &unk : unk1_1)
        SREAD(unk);
    while (!s.eof())
    {
        repeater r;
        r.extract(s);
        unk2.push_back(r);
    }
}

void segment2::repeater::extract(s_file &s)
{
    SREAD(unk2);
    triangles2.resize(unk2);
    SREAD(unk8);
    SREAD(unk3);
    for (auto &t : triangles2)
        SREAD(t);
    SREAD(unk6);
    SREAD(flags);
    SREAD(n_vertex);
    vertices.resize(n_vertex);
    SREAD(n_triangles);
    triangles.resize(n_triangles);
    for (auto &v : vertices)
        v.load(s, flags);
    for (auto &t : triangles)
        SREAD(t);
}

void segment6::extract(s_file &s)
{
    SREAD(name);
    SREAD(unk0);
    triangles.resize(unk0[0][0]);
    for (int i = 0; i < 1; i++)
    {
        for (auto &t : triangles)
            SREAD(t);
        char unk1[0x30]; // some 6 floats
        for (int i = 0; i < unk0[0][0]; i++)
            SREAD(unk1);
    }
}

void model::load(FILE *f)
{
    FREAD(n_fragments);
    FREAD(header);
    fragments.resize(n_fragments);
    for (int i = 0; i < fragments.size(); i++)
    {
        fragments[i].load(f);
        if (!fragments[i].extract())
            throw std::logic_error("extraction error: fragment #" + std::to_string(i));
    }
}

void model::writeObj(std::string fn)
{
    ofstream o(fn);
    o << "# " << "\n";
    o << "# A.I.M. Model Converter (ver. " << version() << ")\n";
    o << "# " << "\n";
    o << "\n";

    if (fragments.empty())
        return;

    const auto &f = fragments[0];
    for (auto &v : f.vertices)
        o << v.printVertex() << "\n";
    o << "\n";
    for (auto &v : f.vertices)
        o << v.printNormal() << "\n";
    o << "\n";
    for (auto &v : f.vertices)
        o << v.printTex() << "\n";
    o << "\n";

    for (int i = 0; i <= f.n_triangles - 3; i += 3)
    {
        auto x = to_string(f.triangles[i] + 1);
        auto y = to_string(f.triangles[i + 2] + 1);
        auto z = to_string(f.triangles[i + 1] + 1);
        x += "/" + x;
        y += "/" + y;
        z += "/" + z;
        o << "f " << x << " " << y << " " << z << "\n";
    }
}
