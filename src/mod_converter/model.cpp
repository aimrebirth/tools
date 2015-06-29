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

#include <common.h>

using namespace std;

std::string version();

void vertex::load(buffer &b, uint32_t flags)
{
    READ(b, vX);
    READ(b, vZ);
    READ(b, vY);

    if (flags & F_WIND)
        READ(b, wind);
    
    READ(b, nX);
    READ(b, nZ);
    READ(b, nY);
    
    READ(b, t1);
    READ(b, t2);
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

void fragment::load(buffer &b)
{
    // header
    READ(b, type);
    READ(b, name0);
    READ(b, name1);
    READ(b, name2);
    READ(b, name3);
    READ(b, name4);
    READ(b, unk0);
    READ(b, unk1);
    READ(b, unk2);
    READ(b, unk3);
    READ(b, size);
    READ(b, unk4);

    // data
    buffer data(b, size);
    READ(data, n_segments);
    segments.resize(n_segments);
    READ(data, header);
    READ(data, triangles_mult_7);
    READ(data, unk10);
    READ(data, flags);
    READ(data, n_vertex);
    vertices.resize(n_vertex);
    READ(data, n_triangles);
    if (triangles_mult_7)
        n_triangles *= 7;
    triangles.resize(n_triangles);
    for (auto &v : vertices)
        v.load(data, flags);
    for (auto &t : triangles)
        READ(data, t);

    // segments
    for (auto &seg : segments)
    {
        uint32_t type;
        READ(data, type);
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
        seg->load(data);
    }

    if (!data.eof())
        throw std::logic_error("extraction error: fragment #" + std::string(name0));
}

void segment1::load(buffer &b)
{
    READ(b, name);
    READ(b, unk0);
    triangles.resize(unk0[0][0]);
    unk1.resize(unk0[0][0]);
    for (int i = 0; i < 2; i++)
    {
        for (auto &t : triangles)
            READ(b, t);
        for (auto &unk: unk1)
            READ(b, unk);
    }
}

void segment2::load(buffer &b)
{
    READ(b, name);
    READ(b, unk0);
    triangles.resize(unk0[0][0]);
    unk1.resize(unk0[0][0]);
    unk1_1.resize(unk0[0][0]);
    for (auto &t : triangles)
        READ(b, t);
    for (auto &unk : unk1)
        READ(b, unk);
    for (auto &unk : unk1_1)
        READ(b, unk);
    while (!b.eof())
    {
        repeater r;
        r.load(b);
        unk2.push_back(r);
    }
}

void segment2::repeater::load(buffer &b)
{
    READ(b, unk2);
    triangles2.resize(unk2);
    READ(b, unk8);
    READ(b, unk3);
    for (auto &t : triangles2)
        READ(b, t);
    READ(b, unk6);
    READ(b, flags);
    READ(b, n_vertex);
    vertices.resize(n_vertex);
    READ(b, n_triangles);
    triangles.resize(n_triangles);
    for (auto &v : vertices)
        v.load(b, flags);
    for (auto &t : triangles)
        READ(b, t);
}

void segment6::load(buffer &b)
{
    READ(b, name);
    READ(b, unk0);
    triangles.resize(unk0[0][0]);
    for (int i = 0; i < 1; i++)
    {
        for (auto &t : triangles)
            READ(b, t);
        char unk1[0x30]; // some 6 floats
        for (int i = 0; i < unk0[0][0]; i++)
            READ(b, unk1);
    }
}

void model::load(buffer &b)
{
    READ(b, n_fragments);
    READ(b, header);
    fragments.resize(n_fragments);
    for (auto &f : fragments)
        f.load(b);
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
