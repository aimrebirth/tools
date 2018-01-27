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

Segment *Segment::create_segment(const buffer &b)
{
    ObjectType segment_type;
    READ(b, segment_type);

    Segment *segment = 0;
    switch (segment_type)
    {
    case ObjectType::ROAD:
        segment = new SegmentObjects<Road>;
        break;
    case ObjectType::BUILDING:
        segment = new SegmentObjects<Building>;
        break;
    case ObjectType::TREE:
        segment = new SegmentObjects<Tree>;
        break;
    case ObjectType::STONE:
        segment = new SegmentObjects<Stone>;
        break;
    case ObjectType::HELPER:
        segment = new SegmentObjects<Helper>;
        break;
    case ObjectType::IMAGE:
        segment = new SegmentObjects<Image>;
        break;
    case ObjectType::LAMP:
        segment = new SegmentObjects<Lamp>;
        break;
    case ObjectType::SOUND:
        segment = new SegmentObjects<Sound>;
        break;
    case ObjectType::ANOMALY:
        segment = new SegmentObjects<Anomaly>;
        break;
    case ObjectType::TOWER:
        segment = new SegmentObjects<Tower>;
        break;
    case ObjectType::BOUNDARY:
        segment = new SegmentObjects<Boundary>;
        break;
    case ObjectType::SOUND_ZONE:
        segment = new SegmentObjects<SoundZone>;
        break;
    case ObjectType::unk0:
        segment = new SegmentObjects<unk0>;
        break;
    case ObjectType::TANK:
        segment = new SegmentObjects<Tank>;
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

void Objects::load(const buffer &b)
{
    uint32_t n_segments = 0;
    READ(b, n_segments);

    for (int s = 0; s < n_segments; s++)
    {
        auto seg = Segment::create_segment(b);
        if (!seg)
            break;
        seg->load(buffer(b, seg->segment_len));
        segments.push_back(seg);
    }
}
