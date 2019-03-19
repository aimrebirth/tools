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

#include <boost/algorithm/string.hpp>
#include <primitives/sw/cl.h>
#include <unicode/translit.h>
#include <unicode/errorcode.h>

#include <iostream>

cl::opt<float> scale_multiplier("s", cl::desc("Model scale multiplier"), cl::init(1.0f));

template <typename T>
inline bool replace_all(T &str, const T &from, const T &to)
{
    bool replaced = false;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != T::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
        replaced = true;
    }
    return replaced;
}

inline bool replace_all(std::string &str, const std::string &from, const std::string &to)
{
    return replace_all<std::string>(str, from, to);
}

std::string version();

float scale_mult()
{
    return scale_multiplier;
}

// UE does not recognize russian strings in .obj
// what about fbx?
// TODO: what to do with signs: soft sign -> ' ?
std::string translate(const std::string &s)
{
    UErrorCode ec = UErrorCode::U_ZERO_ERROR;
    auto tr = icu::Transliterator::createInstance("Lower; Any-Latin; NFC; Latin-ASCII;", UTransDirection::UTRANS_FORWARD, ec);
    if (!tr || ec)
        throw std::runtime_error("Cannot create translator, ec = " + std::to_string(ec));
    icu::UnicodeString s2(s.c_str());
    tr->transliterate(s2);
    std::string s3;
    s2.toUTF8String<std::string>(s3);
    replace_all(s3, " ", "");
    // remove soft signs
    replace_all(s3, "'", "");
    return s3;
}

enum AxisSystem
{
    MayaYUpZFrontRH,
    Windows3dViewer = MayaYUpZFrontRH,

    AIM,
    UE4 = AIM, // Do not use 'Convert scene' during UE4 import!

    ax_aim = AIM,
    ax_ue4 = ax_aim,
    ax_maya_y = MayaYUpZFrontRH,
    ax_win_3d_viewer = ax_maya_y,
};

cl::opt<AxisSystem> AS(cl::desc("Choose axis system:"),
    cl::values(
        clEnumVal(ax_ue4, "Original AIM or UE4 axis system"),
        clEnumVal(ax_maya_y, "Default MAYA Y-Up Z-Front or Windows 3d Viewer axis system")
    )
    , cl::init(AxisSystem::UE4)
);

int get_x_coordinate_id()
{
    switch (AS)
    {
    case AxisSystem::AIM:
        return 1;
    default:
        return 0;
    }
}

static void load_translated(aim_vector3<float> &v, const buffer &b)
{
    /*
    Our coord system:

      ^ z
      |
       ---> y
     /
    v x

    AIM Coordinates:
    1st number: +Y (left) (or -Y (right)?) (-Y?)
    2nd number: +X (front) - 100% sure (-X?)
    3rd number: +Z (up) - 100% sure

    This is Z UP, LH axis system.

    Also see https://twitter.com/FreyaHolmer/status/644881436982575104
    */

    switch (AS)
    {
    case AxisSystem::MayaYUpZFrontRH:
        // Y UP, Z FRONT (RH)
        READ(b, v.x);
        READ(b, v.z);
        READ(b, v.y);
        break;
    case AxisSystem::AIM:
        // Z UP, X FRONT (LH)
        READ(b, v.y);
        READ(b, v.x);
        READ(b, v.z);
        break;
    default:
        throw SW_RUNTIME_ERROR("Unknown Axis System");
    }

    /*
    // Y UP, X FRONT (LH?) (blender accepts such fbx)
    z,x,y
    */
}

void aim_vector4::load(const buffer &b, uint32_t flags)
{
    load_translated(*this, b);
    if (flags & F_WIND_TRANSFORM)
    {
        float f;
        READ(b, f);
    }
}

std::string aim_vector4::print() const
{
    std::string s;
    s += std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z);
    return s;
}

void vertex_normal::load(const buffer &b)
{
    load_translated(*this, b);
}

void uv::load(const buffer &b)
{
    READ(b, u);
    READ(b, v);
    v = 1 - v;
}

void vertex::load(const buffer &b, uint32_t flags)
{
    coordinates.load(b, flags);
    normal.load(b);
    texture_coordinates.load(b);
}

void face::load(const buffer &b)
{
    READ(b, x);
    READ(b, y);
    READ(b, z);
}

static String print_float(double v)
{
    char buf[20];
    snprintf(buf, sizeof(buf), "%.10f", v);
    return buf;
};

std::string vertex::printVertex() const
{
    std::string s;
    s = "v " +
        print_float(coordinates.x * scale_mult())
        + " " + print_float(coordinates.y * scale_mult())
        + " " + print_float(coordinates.z * scale_mult())
        //+ " " + print_float(coordinates.w)
        ;
    return s;
}

std::string vertex::printNormal() const
{
    std::string s;
    s = "vn " + print_float(normal.x) + " " + print_float(normal.y) + " " + print_float(normal.z);
    return s;
}

std::string vertex::printTex() const
{
    std::string s;
    float i;
    auto u = modf(fabs(texture_coordinates.u), &i);
    auto v = modf(fabs(texture_coordinates.v), &i);
    s = "vt " + print_float(u) + " " + print_float(v);
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
        t.load(b);
}

std::string mat_color::print() const
{
    std::string s;
    s += std::to_string(r) + " " + std::to_string(g) + " " + std::to_string(b);
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
    READ(b, type); // seen: 1, 3
    READ_STRING_N(b, name, 0xC);
    for (auto &s : segments)
        s.loadHeader(b);
    if (segments[0].n == 0)
        return;
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
    std::string s;
    s += "newmtl " + h.name + "\n";
    s += "\n";
    s += "Ka " + mat.ambient.print() + "\n";
    s += "Kd " + mat.diffuse.print() + "\n";
    s += "Ks " + mat.specular.print() + "\n";
    s += "   Ns " + std::to_string(mat.power) + "\n";
    // d 1.0
    // illum
    s += "\n";
    if (h.mask.name != "_DEFAULT_")
        s += "map_Ka " + h.mask.name + texture_extension + "\n";
    if (h.mask.name != "_DEFAULT_")
        s += "map_Kd " + h.mask.name + texture_extension + "\n";
    if (h.spec.name != "_DEFAULT_")
        s += "map_Ks " + h.spec.name + texture_extension + "\n";
    if (h.spec.name != "_DEFAULT_")
        s += "map_Ns " + h.spec.name + texture_extension + "\n";
    s += "\n";
    return s;
}

std::string block::printObj(int group_offset) const
{
    std::string s;
    s += "usemtl " + h.name + "\n";
    s += "\n";
    s += "g " + h.name + "\n";
    s += "s 1\n"; // still unk how to use
    s += "\n";

    s += "# " + std::to_string(vertices.size()) + " vertices\n";
    for (auto &v : vertices)
        s += v.printVertex() + "\n";
    s += "\n";
    s += "# " + std::to_string(vertices.size()) + " vertex normals\n";
    for (auto &v : vertices)
        s += v.printNormal() + "\n";
    s += "\n";
    s += "# " + std::to_string(vertices.size()) + " texture coords\n";
    for (auto &v : vertices)
        s += v.printTex() + "\n";
    s += "\n";

    s += "# " + std::to_string(vertices.size()) + " faces\n";
    for (auto &t : faces)
    {
        auto x = std::to_string(t.x + 1 + group_offset);
        auto y = std::to_string(t.y + 1 + group_offset);
        auto z = std::to_string(t.z + 1 + group_offset);
        x += "/" + x + "/" + x;
        y += "/" + y + "/" + y;
        z += "/" + z + "/" + z;
        s += "f " + x + " " + z + " " + y + "\n";
    }

    s += "\n";
    s += "\n";
    return s;
}

void block::header::texture::load(const buffer &b)
{
    READ_STRING(b, name);
    if (gameType == GameType::AimR)
        READ(b, number);
}

void block::header::load(const buffer &b)
{
    READ(b, type);
    READ_STRING(b, name);
    name = translate(name);
    mask.load(b);
    spec.load(b);
    tex3.load(b);
    tex4.load(b);
    READ(b, all_lods);
    if (gameType == GameType::AimR)
    {
        READ(b, unk2[0]);
        READ(b, unk2[1]);
        READ(b, size);
    }
    else
        READ(b, unk2);
    READ(b, unk3);
    if (gameType != GameType::AimR)
        READ(b, size);
    else
        READ(b, unk2[2]); // unk4_0 - related to unk4 - some vector3f
    READ(b, unk4);
}

void block::load(const buffer &b)
{
    h.load(b);

    //if (h.size == 0) // critical error!!! cannot survive
    //    throw std::runtime_error("model file has bad block size field (size == 0)");

    // data
    buffer data = buffer(b, h.size);

    // we cannot process this type at the moment
    if (h.type == BlockType::ParticleEmitter)
        return;
    if (h.type == BlockType::BitmapAlpha)
        return;

    // if we have size - create new buffer
    // else - pass current
    // no copy when buffer is created before
    loadPayload(h.size == 0 ? b : data);
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

    // check mat type - warn if unknown
    switch (mat_type)
    {
    case MaterialType::Texture:
    case MaterialType::TextureWithGlareMap:
    case MaterialType::AlphaTextureNoGlare:
    case MaterialType::AlphaTextureWithOverlap:
    case MaterialType::TextureWithGlareMap2:
    case MaterialType::AlphaTextureDoubleSided:
    case MaterialType::DetalizationObjectGrass:
    case MaterialType::Fire:
    case MaterialType::MaterialOnly:
    case MaterialType::TextureWithDetalizationMap:
    case MaterialType::DetalizationObjectStone:
    case MaterialType::TextureWithDetalizationMapWithoutModulation:
    case MaterialType::TiledTexture:
    case MaterialType::TextureWithGlareMapAndMask:
    case MaterialType::TextureWithMask:
    case MaterialType::Fire2:
        break;
    default:
        std::cout << h.name << ": " << "warning: unknown material type " << (int)mat_type << " \n";
        break;
    }

    // unk
    // seen: 2,3,4,8,9,516
    READ(data, unk7);
    // seen: 0.0222222, 0.0444444, 0.0555556, 0.03125, 0.0375, 0.0625, 0.1, 0.125, 100, inf
    READ(data, unk9); // scale? probably no
    READ(data, unk10);
    READ(data, auto_animation);
    READ(data, animation_cycle);
    READ(data, unk8);
    READ(data, unk11);
    READ(data, unk12);
    READ(data, triangles_mult_7); // particle system?

    /*if (unk7 != 0)
        std::cout << "nonzero unk7 = " << unk7 << " in block " << h.name << "\n";
    if (unk9 != 0)
        std::cout << "nonzero unk9 = " << unk9 << " in block " << h.name << "\n";*/
    //

    READ(data, additional_params);
    uint32_t n_damage_models;
    READ(data, n_damage_models);
    damage_models.resize(n_damage_models);
    READ(data, rot);
    READ(data, flags);
    uint32_t n_vertex;
    uint32_t n_faces; // ??? edges? polygons? triangles?
    READ(data, n_vertex);
    vertices.resize(n_vertex);
    READ(data, n_faces);
    auto n_triangles = n_faces / 3;
    for (auto &v : vertices)
        v.load(data, flags);
    faces.resize(n_triangles);
    for (auto &t : faces)
        t.load(data);

    // animations
    for (auto &a : animations)
        a.load(data);
    for (auto &dm : damage_models)
        dm.load(data);

    auto read_more_faces = [&]()
    {
        n_faces *= 6; // 7

        decltype(faces) faces2;
        faces2.resize(n_faces / 3);
        for (auto &t : faces2)
            READ(data, t);
    };

    // maybe two winds anims?
    // small wind and big wind?
    if (triangles_mult_7 && !unk11 &&
        ((flags & F_WIND_TRANSFORM) || flags == 0x112)
        )
    {
        read_more_faces();
    }

    if (unk7 != 0)
        return;

    std::string s = "extraction error: block: " + std::string(h.name);

    // unknown how to proceed
    if (!data.eof() && triangles_mult_7)
    {
        if (unk11)
        {
            read_more_faces();
        }
        else
        {
            // unknown end of block
            decltype(faces) triangles2;
            auto d = data.end() - data.index();
            triangles2.resize(d / sizeof(face));
            for (auto &t : triangles2)
                READ(data, t);
            uint16_t t;
            while (!data.eof())
                READ(data, t);
        }
    }
    if (!data.eof() && h.size)
        throw std::logic_error(s);
}

bool block::isEngineFx() const
{
    return h.type == BlockType::HelperObject && h.name.find(boost::to_lower_copy(std::string("FIRE"))) == 0;
}

bool block::canPrint() const
{
    // block all lods except 0
    if (!(h.all_lods == 15 || h.LODs.lod1))
        return false;

    // lods
    if (h.type == BlockType::VisibleObject)
        return true;

    // particles
    if (h.type == BlockType::ParticleEmitter)
        return false;

    // collision object
    if (h.name == "SHAPE")
        return false;

    // default
    return false;
}

block::block_info block::save(yaml &root) const
{
    aim_vector4 min{ 1e6, 1e6, 1e6, 1e6 }, max{ -1e6, -1e6, -1e6, -1e6 };
    for (auto &v : vertices)
    {
        auto mm = [&v](auto &m, auto f)
        {
            m.x = f(m.x, v.coordinates.x);
            m.y = f(m.y, v.coordinates.y);
            m.z = f(m.z, v.coordinates.z);
        };

        mm(min, [](auto x, auto y) {return std::min(x,y); });
        mm(max, [](auto x, auto y) {return std::max(x,y); });
    }

    root["xlen"] = max.x - min.x;
    root["ylen"] = max.y - min.y;
    root["zlen"] = max.z - min.z;

    // convex hull length
    /*float len = 0;
    for (auto &v1 : vertices)
    {
        for (auto &v2 : vertices)
        {
            len = std::max(len, sqrt(
                pow(v2.coordinates.x - v1.coordinates.x, 2) +
                pow(v2.coordinates.y - v1.coordinates.y, 2) +
                pow(v2.coordinates.z - v1.coordinates.z, 2)
            ));
        }
    }
    root["len"] = len;*/

    return {min,max};
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

void model::print(const std::string &fn) const
{
    auto title = [](auto &o)
    {
        o << "#" << "\n";
        o << "# A.I.M. Model Converter (ver. " << version() << ")\n";
        o << "#" << "\n";
        o << "\n";
    };

    auto print_obj = [&](const auto &n)
    {
        std::ofstream o(n);
        title(o);
        o << "mtllib " + fn + ".mtl\n\n";
        o << "o " << fn << "\n\n";
        int n_vert = 0;
        for (auto &b : blocks)
        {
            if (!b.canPrint())
                continue;

            o << b.printObj(n_vert) << "\n";
            n_vert += b.vertices.size();
        }
    };

    auto mtl_fn = fn + ".mtl";
    std::ofstream m(mtl_fn);
    title(m);
    for (auto &b : blocks)
        m << b.printMtl() << "\n";

    print_obj(fn + ".obj");
}

void model::save(yaml &root) const
{
    aim_vector4 min{ 1e6, 1e6, 1e6, 1e6 }, max{ -1e6, -1e6, -1e6, -1e6 };

    for (auto &b : blocks)
    {
        if (!b.canPrint())
            continue;

        auto [bmin, bmax] = b.save(root["lods"][b.h.name]);

        auto mm = [](auto &v, auto &m, auto f)
        {
            m.x = f(m.x, v.x);
            m.y = f(m.y, v.y);
            m.z = f(m.z, v.z);
        };

        mm(bmin, min, [](auto x, auto y) {return std::min(x,y); });
        mm(bmax, max, [](auto x, auto y) {return std::max(x,y); });
    }

    root["full"]["xlen"] = max.x - min.x;
    root["full"]["ylen"] = max.y - min.y;
    root["full"]["zlen"] = max.z - min.z;
    //root["full"]["len"] = sqrt(pow(max.x - min.x, 2) + pow(max.y - min.y, 2) + pow(max.z - min.z, 2));
}
