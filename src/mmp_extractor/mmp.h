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

#include <buffer.h>
#include <color.h>
#include <mat.h>
#include <types.h>

#include <primitives/filesystem.h>

#include <map>
#include <set>
#include <stdint.h>
#include <string>
#include <vector>

using Height = float;

enum class HeaderSegmentType : uint32_t
{
    water       =   1,
    weather     =   2,
};

struct header_segment
{
    HeaderSegmentType type;
    uint32_t unk0;
    uint32_t len;

    virtual void load(const buffer &b) = 0;
};

struct water_segment : public header_segment
{
    water_group wg;

    virtual void load(const buffer &b) override;
};

struct weather_segment : public header_segment
{
    weather_group wg;

    virtual void load(const buffer &b) override;
};

struct header
{
    uint32_t unk0;
    std::wstring name1;
    std::wstring name2;
    uint32_t width;
    uint32_t length;
    uint32_t n_header_segs;
    std::string name; // default horizont name?
    std::vector<header_segment*> segments;

    void load(const buffer &b);

private:
    header_segment *create_segment(const buffer &b);
};

// see https://docs.unrealengine.com/latest/INT/Engine/Landscape/TechnicalGuide/index.html
struct segment
{
    static const int len = 65;
    static const int size = len * len;

    static const int mini_len = 33;
    static const int mini_size = mini_len * mini_len;

    struct description
    {
        vector3f min;
        vector3f max;
        float unk0[5];
        uint32_t unk1[7];
    };

    struct info
    {
        uint16_t render_flags;
        uint16_t texture_index;

        uint16_t getTexture() const { return texture_index & 0x0fff; } // first 4 bits are unk (flags?)
    };

    using shadow = color;

    struct normal
    {
        int16_t x;
        int16_t y;
    };

    struct flagged_heightmap
    {
        uint16_t height;
        uint16_t flag;
    };

    struct mini_lod
    {
        flagged_heightmap Heightmap[mini_size];
        color Colormap[mini_size];
        uint32_t unk0[mini_size]; // shadowmap?
        normal unk1[mini_size]; // normals?
    };

    struct data
    {
        uint32_t MagicNumber;
        mini_lod mlod;
        Height Heightmap[size];
        info Infomap[size];
        color Colormap[size]; // diffuse color of material
        shadow Shadowmap[size];
        normal Normalmap[size];
    };

    struct mini_lod2
    {
        flagged_heightmap Heightmap[mini_len][mini_len];
        color Colormap[mini_len][mini_len];
        uint32_t unk0[mini_len][mini_len]; // shadowmap?
        normal unk1[mini_len][mini_len]; // normals?
    };

    struct data2
    {
        uint32_t MagicNumber;
        mini_lod2 mlod;
        Height Heightmap[len][len];
        info Infomap[len][len];
        color Colormap[len][len];
        shadow Shadowmap[len][len];
        normal Normalmap[len][len];
    };

    description desc;
    data d;
    data2 d2;

    void load(const buffer &b);
};

struct mmp
{
    header h;
    std::vector<segment> segments;

    //
    path filename;
    int xsegs;
    int ysegs;
    std::map<int, int /* count */> textures;
    std::map<int, color> textures_map;
    std::map<int, mat<uint32_t>> alpha_maps;
    std::map<int, color> textures_map_colored;
    std::map<int, std::string> textures_names;
    Height h_min = 0;
    Height h_max = 0;
    double scale16 = 0;
    double scale = 0;
    mat<uint16_t> heightmap;
    //mat<uint16_t> heightmap_segmented;
    mat<uint32_t> texmap;
    mat<uint32_t> texmap_colored;
    mat<uint32_t> colormap;
    mat<uint32_t> shadowmap;
    mat<uint32_t> normalmap;

    void load(const buffer &b);
    void load(const path &filename);
    void loadTextureNames(const path &filename);

    void process();

    void writeFileInfo();
    void writeTexturesList();
    void writeHeightMap();
    void writeHeightMapSegmented();
    void writeTextureMap();
    void writeTextureAlphaMaps();
    void writeTextureMapColored();
    void writeColorMap();
    void writeShadowMap();
    void writeNormalMap();
    void writeSplitColormap() const;
};
