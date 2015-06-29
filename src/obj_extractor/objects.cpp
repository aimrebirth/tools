/*
 * AIM obj_extractor
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

#include "objects.h"

Segment *Segment::create_segment(buffer &b)
{
    SegmentType segment_type;
    READ(b, segment_type);

    Segment *segment = 0;
    switch (segment_type)
    {
    case SegmentType::ROAD:
        segment = new SegmentObjects<Road>;
        break;
    case SegmentType::BUILDING:
        segment = new SegmentObjects<Building>;
        break;
    case SegmentType::SURFACE:
        segment = new SegmentObjects<Surface>;
        break;
    case SegmentType::STONE:
        segment = new SegmentObjects<Stone>;
        break;
    case SegmentType::HELPER:
        segment = new SegmentObjects<Helper>;
        break;
    case SegmentType::SHELL:
        segment = new SegmentObjects<Shell>;
        break;
    case SegmentType::IMAGE:
        segment = new SegmentObjects<Image>;
        break;
    case SegmentType::EXPLOSION:
        segment = new SegmentObjects<Explosion>;
        break;
    case SegmentType::SOUND:
        segment = new SegmentObjects<Sound>;
        break;
    case SegmentType::MUSIC:
        segment = new SegmentObjects<Music>;
        break;
    case SegmentType::ANOMALY:
        segment = new SegmentObjects<Anomaly>;
        break;
    case SegmentType::TOWER:
        segment = new SegmentObjects<Tower>;
        break;
    case SegmentType::BOUNDARY:
        segment = new SegmentObjects<Boundary>;
        break;
    case SegmentType::GOODS:
        segment = new SegmentObjects<Goods>;
        break;
    case SegmentType::unk1:
        segment = new SegmentObjects<unk1>;
        break;
    default:
        assert(false);
        break;
    }
    if (segment)
    {
        segment->segment_type = segment_type;
        READ(b, segment->segment_len);
        READ(b, segment->n_objects);
    }
    return segment;
}

void Objects::load(buffer &b)
{
    READ(b, n_segments);

    for (int s = 0; s < n_segments; s++)
    {
        auto seg = Segment::create_segment(b);
        if (!seg)
            break;
        seg->load(b);
        segments.push_back(seg);
    }
}
