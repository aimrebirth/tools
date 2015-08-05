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

#include "mmp.h"

void unk_segment1::load(buffer &b)
{
    while (!b.eof())
    {
        unk_segment1_1 s;
        s.load(b);
        segments.push_back(s);
    }
}

void unk_segment1_1::load(buffer &b)
{
    READ(b, unk0);
    READ(b, name1);
    READ(b, unk1);
    READ(b, unk2);
    READ(b, unk3);
    READ(b, unk4);
    READ(b, name2);
    READ(b, unk5);
}

void unk_segment2::load(buffer &b)
{
    READ(b, n_segs);
    segments.resize(n_segs);
    READ(b, name);
    for (auto &s : segments)
        s.load(b);
}

void unk_segment2_1::load(buffer &b)
{
    READ(b, name1);
    READ(b, name2);
    READ(b, name3);
    READ(b, name4);
    READ(b, name5);
    READ(b, unk0);
    READ(b, unk1);
    READ(b, tex_name0);
    READ(b, unk2);
    READ(b, unk_name0);
    READ(b, unk3);
    READ(b, tex_name1);
    READ(b, tex_name2);
    READ(b, unk4);
}

header_segment *header::create_segment(buffer &b)
{
    HeaderSegmentType type;
    READ(b, type);

    header_segment *segment = 0;
    switch (type)
    {
    case HeaderSegmentType::unk1:
        segment = new unk_segment1;
        break;
    case HeaderSegmentType::unk2:
        segment = new unk_segment2;
        break;
    default:
        throw std::logic_error("unknown header segment type " + std::to_string((int)type));
        break;
    }
    if (segment)
    {
        segment->type = type;
        READ(b, segment->unk0);
        READ(b, segment->len);
    }
    return segment;
}

void header::load(buffer &b)
{
    READ(b, unk0);
    READ(b, name1);
    READ(b, name2);
    READ(b, width);
    READ(b, height);
    READ(b, n_segs);
    segments.resize(n_segs);
    READ(b, name);
    for (auto &s : segments)
    {
        s = create_segment(b);
        buffer b2(b, s->len);
        if (!b2.eof())
            s->load(b2);
    }
}

void segment::load(buffer &b)
{
    READ(b, desc);
    buffer b2(b);
    b2.seek(desc.offset);
    READ(b2, d);
}

void mmp::load(buffer &b)
{
    h.load(b);
    int n_segs = h.width / 64 * h.height / 64;
    segments.resize(n_segs);
    for (auto &s : segments)
        s.load(b);
}