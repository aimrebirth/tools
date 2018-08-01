/*
 * AIM mpj_loader
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

#include "mpj.h"

#include <algorithm>
#include <assert.h>
#include <fstream>
#include <iomanip>

#include <primitives/filesystem.h>

segment *segment::create_segment(const buffer &b)
{
    SegmentType type;
    READ(b, type);
    if (type == SegmentType::none)
        READ(b, type);

    segment *segment = nullptr;
    switch (type)
    {
    case SegmentType::MapData:
        segment = new map_data;
        break;
    case SegmentType::Surface:
        segment = new surface;
        break;
    case SegmentType::Weather:
        segment = new weather_data;
        break;
    case SegmentType::Objects:
        segment = new objects_data;
        break;
    case SegmentType::Water:
        segment = new water_data;
        break;
    case SegmentType::unk7:
        segment = new segment7;
        break;
    case SegmentType::unk9:
        segment = new segment9;
        break;
    case SegmentType::BuildingGoods:
        segment = new building_goods;
        break;
    case SegmentType::MapMusic:
        segment = new map_music;
        break;
    case SegmentType::Organizations:
        segment = new organizations;
        break;
    case SegmentType::Goods:
        segment = new gliders_n_goods;
        break;
    default:
        assert(false);
        break;
    }
    if (segment)
    {
        segment->type = type;
        READ(b, segment->unk0);
        READ(b, segment->size);
    }
    return segment;
}

void map_data::load(const buffer &b)
{
    auto sz = b.size();
    assert(sz % 3 == 0);
    sz /= 3;
#define READ_SEG(v) \
    v.resize(sz / sizeof(decltype(v)::value_type)); \
    READ_N(b, v[0], v.size())

    READ_SEG(unk1);
    READ_SEG(unk2);
    READ_SEG(unk3);
#undef READ_SEG
}

void surface::load(const buffer &b)
{
    while (!b.eof())
    {
        value v;
        READ_STRING(b, v.name);
        READ(b, v.unk0);
        unk1.push_back(v);
    }
}

void weather_data::load(const buffer &b)
{
    wg.load(b);
}

void objects_data::load(const buffer &b)
{
    objects.load(b);
}

void segment7::load(const buffer &b)
{
    auto n = b.size() / sizeof(decltype(data)::value_type);
    data.resize(n);
    READ_N(b, data[0], n);
}

void water_data::load(const buffer &b)
{
    wg.load(b);
}

void segment9::load(const buffer &b)
{
    auto n = b.size() / sizeof(decltype(data)::value_type);
    data.resize(n);
    READ_N(b, data[0], n);
}

void building_goods::load(const buffer &b)
{
    READ(b, unk1);
    READ(b, n);
    bgs.resize(n);
    for (auto &bg : bgs)
        bg.load(b);
}

void map_music::load(const buffer &b)
{
    while (!b.eof())
    {
        MapMusic mm;
        mm.load(b);
        mms.push_back(mm);
        READ(b, unk1); // or read after cycle?
    }
}

void organizations::load(const buffer &b)
{
    READ(b, n);
    orgs.resize(n);
    for (auto &org : orgs)
        org.load(b);
    bases.load(b);
}

void gliders_n_goods::load(const buffer &b)
{
    READ(b, n_good_groups);
    READ(b, n_gliders);
    gliders.resize(n_gliders);
    for (auto &g : gliders)
        g.load(b);
    READ(b, unk1);
    goods.resize(n_good_groups);
    for (auto &g : goods)
        g.load(b);
}

void header::load(const buffer &b)
{
    READ(b, magic);
    if (memcmp(magic, "MPRJ", 4) != 0)
        throw std::runtime_error("This file is not a mechanoid project");
    READ(b, unk0);
    READ(b, unk1);
    READ(b, unk2);
    READ(b, unk3);
    READ(b, width);
    READ(b, height);
    READ(b, unk4);

    auto seg_size = 64;
    auto xsegs = width / seg_size;
    if ((width - 1) % 64 != 0)
        xsegs++;
    auto ysegs = height / seg_size;
    if ((width - 1) % 64 != 0)
        ysegs++;

    while (!b.eof())
    {
        auto seg = segment::create_segment(b);
        if (!seg)
            break;
        seg->load(buffer(b, seg->size));
        segments.push_back(seg);
    }

    if (!b.eof())
    {
        throw std::logic_error("End of file was not reached");
    }
}

void mpj::load(const buffer &b)
{
    h.load(b);
}

void mpj::load(const path &fn)
{
    filename = fn;
    buffer b(read_file(filename));
    load(b);
}