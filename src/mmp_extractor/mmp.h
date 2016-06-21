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

#include <map>
#include <set>
#include <stdint.h>
#include <string>
#include <vector>

#include <buffer.h>
#include <color.h>
#include <mat.h>
#include <types.h>

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

    virtual void load(buffer &b) = 0;
};

struct water_segment : public header_segment
{
    water_group wg;

    virtual void load(buffer &b) override;
};

struct weather_segment : public header_segment
{
    weather_group wg;

    virtual void load(buffer &b) override;
};

struct header
{
    uint32_t unk0;
    std::wstring name1;
    std::wstring name2;
    uint32_t width;
    uint32_t height;
    uint32_t n_header_segs;
    std::string name;
    std::vector<header_segment*> segments;

    void load(buffer &b);

private:
    header_segment *create_segment(buffer &b);
};

struct segment
{
    static const int len = 65;
    static const int size = len * len;

    struct description
    {
        uint32_t offset;
        float Xmin;
        float Ymin;
        float Zmin;
        float Xmax;
        float Ymax;
        float Zmax;
        float unk0[5];
        uint32_t unk1[7];
    };
    struct data
    {
        struct info
        {
            uint16_t render_flags;
            uint16_t texture_index;

            uint16_t getTexture() const { return texture_index & 0x0fff; } // first 4 bits are unk (flags?)
        };
        struct shadow
        {
            uint8_t unk[4];
        };
        struct normal
        {
            int16_t x;
            int16_t y;
        };
        struct mini_lod
        {
            static const int len = 33;
            static const int size = len * len;

            struct flagged_heightmap
            {
                uint16_t height;
                uint16_t flag;
            };

            flagged_heightmap Heightmap[size];
            color Colormap[size];
            uint32_t unk0[size]; // shadowmap?
            normal unk1[size]; // normals?
        };

        uint32_t MagicNumber;
        mini_lod mlod;
        Height Heightmap[size];
        info Infomap[size];
        color Colormap[size];
        shadow Shadowmap[size];
        normal Normalmap[size];
    };

    description desc;
    data d;

    void load(buffer &b);
};

struct mmp
{
    header h;
    std::vector<segment> segments;

    //
    std::string filename;
    int xsegs;
    int ysegs;
    std::map<int, int /* count */> textures;
    std::map<int, color> textures_map;
    std::map<int, mat<uint32_t>> alpha_maps;
    std::map<int, color> textures_map_colored;
    std::map<int, std::string> textures_names;
    Height h_min;
    Height h_max;
    double scale16 = 0;
    mat<uint16_t> heightmap;
    mat<uint32_t> texmap;
    mat<uint32_t> texmap_colored;
    mat<uint32_t> colormap;

    void load(buffer &b);
    void load(const std::string &filename);
    void loadTextureNames(const std::string &filename);

    void process();

    void writeFileInfo();
    void writeTexturesList();
    void writeHeightMap();
    void writeTextureMap();
    void writeTextureAlphaMaps();
    void writeTextureMapColored();
    void writeColorMap();
};
