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

#include <algorithm>
#include <fstream>
#include <set>
#include <string>

#include <buffer.h>

#include <iostream>

using namespace std;

std::string version();

std::string Vector4::print() const
{
    string s;
    s += to_string(x) + " " + to_string(y) + " " + to_string(z);
    return s;
}

void vertex::load(const buffer &b, uint32_t flags)
{
    READ(b, vX);
    READ(b, vZ);
    READ(b, vY);

    if (flags & F_UNK0)
        READ(b, unk0);

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

void damage_model::load(const buffer &b)
{
    READ(b, n_polygons);
    model_polygons.resize(n_polygons);
    READ(b, unk8);
    READ(b, name);
    for (auto &t : model_polygons)
        READ(b, t);
    READ(b, unk6);
    READ(b, flags);
    READ(b, n_vertex);
    vertices.resize(n_vertex);
    READ(b, n_triangles);
    damage_triangles.resize(n_triangles / 3);
    for (auto &v : vertices)
        v.load(b, flags);
    for (auto &t : damage_triangles)
        READ(b, t);
}

void animation::load(const buffer &b)
{
    READ(b, type);
    READ(b, name);
    for (auto &s : segments)
        s.loadHeader(b);
    //if (segments[0].n)
    for (auto &s : segments)
        s.loadData(b);
}

void animation::segment::loadHeader(const buffer &b)
{
    READ(b, n);
    READ(b, unk0);
    READ(b, unk1);
}

void animation::segment::loadData(const buffer &b)
{
    if (n == 0)
        return;
    if (unk0)
    {
        model_polygons.resize(n);
        for (auto &t : model_polygons)
            READ(b, t);
    }
    unk2.resize(n);
    for (auto &unk : unk2)
        READ(b, unk);
}

std::string block::printMtl(const std::string &mtl_name) const
{
    string s;
    s += "newmtl " + mtl_name + "\n";
    s += "\n";
    s += "Ka " + material.ambient.print() + "\n";
    s += "Kd " + material.diffuse.print() + "\n";
    s += "Ks " + material.specular.print() + "\n";
    s += "   Ns " + to_string(material.power) + "\n";
    // d 1.0
    // illum
    s += "\n";
    if (string(tex_mask) != "_DEFAULT_")
        s += "map_Ka " + string(tex_mask) + ".tga" + "\n";
    if (string(tex_mask) != "_DEFAULT_")
        s += "map_Kd " + string(tex_mask) + ".tga" + "\n";
    if (string(tex_spec) != "_DEFAULT_")
        s += "map_Ks " + string(tex_spec) + ".tga" + "\n";
    if (string(tex_spec) != "_DEFAULT_")
        s += "map_Ns " + string(tex_spec) + ".tga" + "\n";
    s += "\n";
    return s;
}

std::string block::printObj(const std::string &mtl_name) const
{
    string s;
    // UE does not recognize russian strings in .obj
    //s += string("o ") + name + "\n";
    //s += string("g ") + name + "\n";
    s += "g group1\n";
    s += "s off\n";
    s += "\n";
    s += "usemtl " + mtl_name + "\n";
    s += "\n";

    for (auto &v : vertices)
        s += v.printVertex() + "\n";
    s += "\n";
    for (auto &v : vertices)
        s += v.printNormal() + "\n";
    s += "\n";
    for (auto &v : vertices)
        s += v.printTex() + "\n";
    s += "\n";

    if (n_triangles)
    {
        for (auto &t : triangles)
        {
            auto x = to_string(t.x + 1);
            auto y = to_string(t.y + 1);
            auto z = to_string(t.z + 1);
            x += "/" + x;
            y += "/" + y;
            z += "/" + z;
            s += "f " + x + " " + y + " " + z + "\n";
        }
    }

    s += "\n";
    s += "\n";
    return s;
}

void block::load(const buffer &b)
{
    // header
    READ(b, type);
    READ_STRING(b, name);
    READ_STRING(b, tex_mask);
    READ_STRING(b, tex_spec);
    READ_STRING(b, tex3);
    READ_STRING(b, tex4);
    READ(b, LODs);
    READ(b, unk2);
    READ(b, unk3);
    READ(b, size);
    READ(b, unk4);

    if (size == 0) // critical error!!! cannot survive
        throw std::runtime_error("model file has bad block size field (size == 0)");

    // data
    buffer data = buffer(b, size);

    // we cannot process this type at the moment
    if (type == BlockType::ParticleEmitter)
        return;

    READ(data, n_animations);
    animations.resize(n_animations);
    READ(data, material);

    // unk
    READ(data, effect);
    READ(data, unk7);
    READ(data, unk9);
    READ(data, unk10);
    READ(data, auto_animation);
    READ(data, animation_cycle);
    READ(data, unk8);
    READ(data, unk11);
    READ(data, unk12);
    READ(data, triangles_mult_7);
    //

    READ(data, additional_params);
    READ(data, n_damage_models);
    damage_models.resize(n_damage_models);
    READ(data, rot);
    READ(data, flags);
    READ(data, n_vertex);
    vertices.resize(n_vertex);
    READ(data, n_triangles);
    if (triangles_mult_7 && (flags & F_UNK0) && !unk11)
        n_triangles *= 7;
    triangles.resize(n_triangles / 3);
    for (auto &v : vertices)
        v.load(data, flags);
    for (auto &t : triangles)
        READ(data, t);

    // animations
    for (auto &a : animations)
        a.load(data);
    for (auto &dm : damage_models)
        dm.load(data);

    string s = "extraction error: block #" + std::string(name);
    if (!data.eof())
    {
        cerr << s << "\n";
        return;
    }

    // unknown how to proceed
    if (!data.eof() && triangles_mult_7)
    {
        // unknown end of block
        auto triangles2 = triangles;
        triangles2.resize((data.size() - data.index()) / sizeof(triangle));
        for (auto &t : triangles2)
            READ(data, t);
    }
    if (!data.eof())
        throw std::logic_error(s);
}

void model::load(const buffer &b)
{
    READ(b, n_blocks);
    if (n_blocks > 1000) // probably bad file
        throw std::runtime_error("Model file has bad block count (should be <= 1000). Probably not a model.");
    READ(b, header);
    blocks.resize(n_blocks);
    for (auto &f : blocks)
        f.load(b);
}
