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

enum class WeatherType : uint32_t
{
    rain    =   0x1,
    snow    =   0x2,
    storm   =   0x4,
};

enum class SmokeType : uint32_t
{
    none,
    exp,
    biexp,
    linear,
};

struct direction
{
    float x;
    float y;
    float z;
};

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

struct water
{
    float unk0[6];
    char name1[0x20];
    uint32_t unk1;
    float unk2;
    uint32_t unk3[16];
    float unk4;
    char name2[0x20];
    uint32_t unk5[16];

    void load(buffer &b);
};

struct water_segment : public header_segment
{
    std::vector<water> segments;

    virtual void load(buffer &b) override;
};

struct weather
{
    struct atmospheric_effects
    {
        direction wind;
        WeatherType weatherType;
        float strength;
        float duration;
        float probability;
    };

    char name[0x20];
    char unk0[0x20];
    uint32_t unk1[2];
    color smoke_1; //3?
    color smoke_3; //1?
    SmokeType smokeType;
    uint32_t unk2[3];
    char cloud_layer1[0x20];
    char cloud_layer2[0x20];
    float cloud_layer1_speed;
    float cloud_layer2_speed;
    direction cloud_layer1_direction;
    direction cloud_layer2_direction;
    char sun[0x20];
    color general_color;
    color sun_color;
    color moon_color;
    char moon[0x20];
    float probability;
    char day_night_gradient_name[0x20];
    char dawn_dusk_gradient_name[0x20];
    color dawn_dusk_color;
    atmospheric_effects effects;
    color smoke_2;
    color smoke_4;
    uint32_t slider_3;
    uint32_t slider_1;
    float unk8[11];

    void load(buffer &b);
};

struct weather_segment : public header_segment
{
    uint32_t n_segs;
    char name[0xA0];
    std::vector<weather> segments;

    virtual void load(buffer &b) override;
};

struct header
{
    uint32_t unk0;
    wchar_t name1[0x20];
    wchar_t name2[0x20];
    uint32_t width;
    uint32_t height;
    uint32_t n_header_segs;
    char name[0xA0];
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
        struct old_data
        {
            uint16_t Heightmap[1089 * 2];
            color Colormap[1089];
            uint32_t unk0[1089];
            normal unk1[1089]; // normals?
        };

        uint32_t MagicNumber;
        old_data old;
        float Heightmap[size];
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
    float h_min;
    float h_max;
    float scale16 = 0;
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
