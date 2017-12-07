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
#include <map>
#include <set>
#include <string>
#include <math.h>

#include <buffer.h>

//#include <Eigen/Core>
//#include <Eigen/Dense>

#include <iostream>

using namespace std;

const float scale_mult = 30.0f;
const std::string texture_extension = ".TM.bmp";

const map<char, string> transliteration =
{
    { 'а',"a" },
    { 'б',"b" },
    { 'в',"v" },
    { 'г',"g" },
    { 'д',"d" },
    { 'е',"e" },
    { 'ё',"yo" },
    { 'ж',"zh" },
    { 'з',"z" },
    { 'и',"i" },
    { 'й',"y" },
    { 'к',"k" },
    { 'л',"l" },
    { 'м',"m" },
    { 'н',"n" },
    { 'о',"o" },
    { 'п',"p" },
    { 'р',"r" },
    { 'с',"s" },
    { 'т',"t" },
    { 'у',"u" },
    { 'ф',"f" },
    { 'х',"kh" },
    { 'ц',"ts" },
    { 'ч',"ch" },
    { 'ш',"sh" },
    { 'щ',"shch" },
    { 'ъ',"_" },
    { 'ы',"y" },
    { 'ь',"_" },
    { 'э',"e" },
    { 'ю',"yu" },
    { 'я',"ya" },

    { 'А',"A" },
    { 'Б',"B" },
    { 'В',"V" },
    { 'Г',"G" },
    { 'Д',"D" },
    { 'Е',"E" },
    { 'Ё',"Yo" },
    { 'Ж',"Zh" },
    { 'З',"Z" },
    { 'И',"I" },
    { 'Й',"Y" },
    { 'К',"K" },
    { 'Л',"L" },
    { 'М',"M" },
    { 'Н',"N" },
    { 'О',"O" },
    { 'П',"P" },
    { 'Р',"R" },
    { 'С',"S" },
    { 'Т',"T" },
    { 'У',"U" },
    { 'Ф',"F" },
    { 'Х',"Kh" },
    { 'Ц',"Ts" },
    { 'Ч',"Ch" },
    { 'Ш',"Sh" },
    { 'Щ',"Shch" },
    { 'Ъ',"_" },
    { 'Ы',"Y" },
    { 'Ь',"_" },
    { 'Э',"E" },
    { 'Ю',"Yu" },
    { 'Я',"Ya" },

    { ' ',"_" },
}
;

std::string version();

// UE does not recognize russian strings in .obj
string translate(const string &s)
{
    string o;
    for (auto c : s)
    {
        auto i = transliteration.find(c);
        if (i == transliteration.end())
            o += c;
        else
            o += i->second;
    }
    return o;
}

template <typename T>
void aim_vector3<T>::load(const buffer &b)
{
    READ(b, x);
    READ(b, z);
    READ(b, y);
}

void aim_vector4::load(const buffer &b, uint32_t flags)
{
    aim_vector3::load(b);
    if (flags & F_USE_W_COORDINATE)
        READ(b, w);
}

std::string aim_vector4::print() const
{
    string s;
    s += to_string(x) + " " + to_string(y) + " " + to_string(z);
    return s;
}

void vertex::load(const buffer &b, uint32_t flags)
{
    coordinates.load(b, flags);
    READ(b, normal);
    READ(b, texture_coordinates);
}

std::string vertex::printVertex(bool rotate_x_90) const
{
    // rotate by 90 grad over Ox axis
/*#define M_PI_2     1.57079632679489661923
    Eigen::Vector3f x;
    x << -coordinates.x, coordinates.y, -coordinates.z;

    Eigen::AngleAxis<float> rx(M_PI_2, Eigen::Vector3f(1, 0, 0));
    auto x2 = rx * x;*/

    string s;
    if (rotate_x_90)
    {
        // that rotation is really equivalent to exchanging y and z and z sign
        s = "v " +
            to_string(-coordinates.x * scale_mult) + " " +
            to_string(coordinates.z * scale_mult) + " " +
            to_string(coordinates.y * scale_mult) + " " +
            to_string(coordinates.w)
            ;
    }
    else
    {
        s = "v " +
            to_string(-coordinates.x * scale_mult) + " " +
            to_string(coordinates.y * scale_mult) + " " +
            to_string(-coordinates.z * scale_mult) + " " +
            to_string(coordinates.w)
            ;
    }
    return s;
}

std::string vertex::printNormal(bool rotate_x_90) const
{
    string s;
    if (rotate_x_90)
        s = "vn " + to_string(-normal.x) + " " + to_string(-normal.z) + " " + to_string(normal.y);
    else
        s = "vn " + to_string(-normal.x) + " " + to_string(normal.y) + " " + to_string(-normal.z);
    return s;
}

std::string vertex::printTex() const
{
    string s;
    float i;
    auto u = modf(fabs(texture_coordinates.u), &i);
    auto v = modf(fabs(texture_coordinates.v), &i);
    s = "vt " + to_string(u) + " " + to_string(1 - v);
    return s;
}

void damage_model::load(const buffer &b)
{
    uint32_t n_polygons;
    READ(b, n_polygons);
    model_polygons.resize(n_polygons);
    READ(b, unk8);
    READ_STRING_N(b, name, 0x3C);
    for (auto &t : model_polygons)
        READ(b, t);
    READ(b, unk6);
    READ(b, flags);
    uint32_t n_vertex;
    uint32_t n_faces;
    READ(b, n_vertex);
    vertices.resize(n_vertex);
    READ(b, n_faces);
    faces.resize(n_faces / 3);
    for (auto &v : vertices)
        v.load(b, flags);
    for (auto &t : faces)
        READ(b, t);
}

std::string mat_color::print() const
{
    string s;
    s += to_string(r) + " " + to_string(g) + " " + to_string(b);
    return s;
}

void material::load(const buffer &b)
{
    READ(b, diffuse);
    READ(b, ambient);
    READ(b, specular);
    READ(b, emissive);
    READ(b, power);

    auto delim_by_3 = [](auto &v)
    {
        //if (v.r > 1)
            //v.r /= 3.0f;
        //if (v.g > 1)
            //v.g /= 3.0f;
        //if (v.b > 1)
            //v.b /= 3.0f;
    };

    // in aim - those values lie in interval [0,3] instead of [0,1]
    delim_by_3(diffuse);
    delim_by_3(ambient);
    delim_by_3(specular);
}

void animation::load(const buffer &b)
{
    READ(b, type);
    READ_STRING_N(b, name, 0xC);
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

std::string block::printMtl() const
{
    string s;
    s += "newmtl " + h.name + "\n";
    s += "\n";
    s += "Ka " + mat.ambient.print() + "\n";
    s += "Kd " + mat.diffuse.print() + "\n";
    s += "Ks " + mat.specular.print() + "\n";
    s += "   Ns " + to_string(mat.power) + "\n";
    // d 1.0
    // illum
    s += "\n";
    if (h.tex_mask != "_DEFAULT_")
        s += "map_Ka " + h.tex_mask + texture_extension + "\n";
    if (h.tex_mask != "_DEFAULT_")
        s += "map_Kd " + h.tex_mask + texture_extension + "\n";
    if (h.tex_spec != "_DEFAULT_")
        s += "map_Ks " + h.tex_spec + texture_extension + "\n";
    if (h.tex_spec != "_DEFAULT_")
        s += "map_Ns " + h.tex_spec + texture_extension + "\n";
    s += "\n";
    return s;
}

std::string block::printObj(int group_offset, bool rotate_x_90) const
{
    string s;
    s += "usemtl " + h.name + "\n";
    s += "\n";
    s += "g " + h.name + "\n";
    s += "s 1\n"; // still unk how to use
    s += "\n";

    for (auto &v : vertices)
        s += v.printVertex(rotate_x_90) + "\n";
    s += "\n";
    for (auto &v : vertices)
        s += v.printNormal(rotate_x_90) + "\n";
    s += "\n";
    for (auto &v : vertices)
        s += v.printTex() + "\n";
    s += "\n";

    for (auto &t : faces)
    {
        auto x = to_string(t.x + 1 + group_offset);
        auto y = to_string(t.y + 1 + group_offset);
        auto z = to_string(t.z + 1 + group_offset);
        x += "/" + x + "/" + x;
        y += "/" + y + "/" + y;
        z += "/" + z + "/" + z;
        s += "f " + x + " " + z + " " + y + "\n";
    }

    s += "\n";
    s += "\n";
    return s;
}

void block::header::load(const buffer &b)
{
    READ(b, type);
    READ_STRING(b, name);
    name = translate(name);
    READ_STRING(b, tex_mask);
    READ_STRING(b, tex_spec);
    READ_STRING(b, tex3);
    READ_STRING(b, tex4);
    READ(b, all_lods);
    READ(b, unk2);
    READ(b, unk3);
    READ(b, size);
    READ(b, unk4);
}

void block::load(const buffer &b)
{
    h.load(b);

    if (h.size == 0) // critical error!!! cannot survive
        throw std::runtime_error("model file has bad block size field (size == 0)");

    // data
    buffer data = buffer(b, h.size);

    // we cannot process this type at the moment
    if (h.type == BlockType::ParticleEmitter)
        return;

    loadPayload(data);
}

void block::loadPayload(const buffer &data)
{
    // anims
    uint32_t n_animations;
    READ(data, n_animations);
    animations.resize(n_animations);

    // mat
    mat.load(data);
    READ(data, mat_type);

    // unk
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
    uint32_t n_damage_models;
    READ(data, n_damage_models);
    damage_models.resize(n_damage_models);
    READ(data, rot);
    READ(data, flags);
    uint32_t n_vertex;
    uint32_t n_faces;
    READ(data, n_vertex);
    vertices.resize(n_vertex);
    READ(data, n_faces);
    if (triangles_mult_7 && (flags & F_USE_W_COORDINATE) && !unk11)
        n_faces *= 7;
    for (auto &v : vertices)
        v.load(data, flags);
    faces.resize(n_faces / 3);
    for (auto &t : faces)
        READ(data, t);

    // animations
    for (auto &a : animations)
        a.load(data);
    for (auto &dm : damage_models)
        dm.load(data);

    string s = "extraction error: block #" + std::string(h.name);
    if (!data.eof())
    {
        cerr << s << "\n";
        return;
    }

    // unknown how to proceed
    if (!data.eof() && triangles_mult_7)
    {
        // unknown end of block
        auto triangles2 = faces;
        triangles2.resize((data.size() - data.index()) / sizeof(face));
        for (auto &t : triangles2)
            READ(data, t);
    }
    if (!data.eof())
        throw std::logic_error(s);
}

bool block::isEngineFx() const
{
    return h.type == BlockType::HelperObject && h.name.find("FIRE") == 0;
}

bool block::canPrint() const
{
    // block all lods except 0
    if (!(h.all_lods == 15 || h.LODs.lod1))
        return false;

    // lods
    if (h.type == BlockType::VisibleObject)
        return true;

    if (isEngineFx())
        return true;

    // particles
    if (h.type == BlockType::ParticleEmitter)
        return false;

    // default
    return false;
}

void model::load(const buffer &b)
{
    int n_blocks;
    READ(b, n_blocks);
    if (n_blocks > 1000) // probably bad file
        throw std::runtime_error("Model file has bad block count (should be <= 1000). Probably not a model.");
    char header[0x40];
    READ(b, header);
    blocks.resize(n_blocks);
    for (auto &f : blocks)
        f.load(b);
}

void model::print(const std::string &fn)
{
    auto title = [](auto &o)
    {
        o << "#" << "\n";
        o << "# A.I.M. Model Converter (ver. " << version() << ")\n";
        o << "#" << "\n";
        o << "\n";
    };

    auto print_obj = [&](const auto &n, bool rotate_x_90 = false)
    {
        ofstream o(n);
        title(o);
        o << "mtllib " + fn + ".mtl\n\n";
        o << "o " << fn << "\n\n";
        int n_vert = 0;
        for (auto &b : blocks)
        {
            if (!b.canPrint())
                continue;

            o << b.printObj(n_vert, rotate_x_90) << "\n";
            n_vert += b.vertices.size();
        }
    };

    auto mtl_fn = fn + ".mtl";
    ofstream m(mtl_fn);
    title(m);
    for (auto &b : blocks)
        m << b.printMtl() << "\n";

    print_obj(fn + "_fbx.obj");
    print_obj(fn + "_ue4.obj", true);
}
