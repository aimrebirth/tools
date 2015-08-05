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

#include <stdint.h>
#include <string>
#include <vector>

#include <common.h>

enum class HeaderSegmentType : uint32_t
{
    unk1    =   1,
    unk2    =   2,
};

struct header_segment
{
    HeaderSegmentType type;
    uint32_t unk0;
    uint32_t len;

    virtual void load(buffer &b) = 0;
};

struct unk_segment1_1
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

struct unk_segment1 : public header_segment
{
    std::vector<unk_segment1_1> segments;

    virtual void load(buffer &b) override;
};

struct unk_segment2_1
{
    char name1[0x20];
    char name2[0x20];
    char name3[0x20];
    char name4[0x20];
    char name5[0x20];
    float unk0[2];
    uint32_t unk1[6];
    char tex_name0[0x20];
    uint32_t unk2[3];
    char unk_name0[0x20];
    float unk3;
    char tex_name1[0x20];
    char tex_name2[0x20];
    uint32_t unk4[23];

    void load(buffer &b);
};

struct unk_segment2 : public header_segment
{
    uint32_t n_segs;
    char name[0xA0];
    std::vector<unk_segment2_1> segments;

    virtual void load(buffer &b) override;
};

struct header
{
    uint32_t unk0;
    wchar_t name1[0x20];
    wchar_t name2[0x20];
    uint32_t width;
    uint32_t height;
    uint32_t n_segs;
    char name[0xA0];
    std::vector<header_segment*> segments;

    void load(buffer &b);

private:
    header_segment *create_segment(buffer &b);
};

struct segment
{
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
        };
        struct color
        {
            uint8_t b;
            uint8_t g;
            uint8_t r;
            uint8_t a;
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
        float Heightmap[4225];
        info Infomap[4225];
        color Colormap[4225];
        shadow Shadowmap[4225];
        normal Normalmap[4225];
    };

    description desc;
    data d;

    void load(buffer &b);
};

struct mmp
{
    header h;
    std::vector<segment> segments;

    void load(buffer &b);
};