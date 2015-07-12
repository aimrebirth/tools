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

#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <vector>

#include <common.h>

using namespace std;

enum class SegmentType : uint32_t
{
    TEXTURE         =   0x1,
    MODEL,
    SURFACE, // stones
    STONE, // trees
    TREE,

    GLIDER,
    HELPER,
    ROAD,
    WEAPON,
    CONFIG,

    SHELL, // buildings
    IMAGE,
    EXPLOSION, // road lights
    EQUIPMENT,
    ORGANIZATION,

    BUILDING,
    LAMP,
    COVERING,
    SOUND,
    MUSIC,

    GOODS,
    ANOMALY,
    TOWER,
    BOUNDARY,
    SOUND_ZONE,

    unk1 = 0x1b,
};

struct Segment
{
    SegmentType segment_type;
    uint32_t    segment_len = 0;
    uint32_t    n_objects = 0;

    virtual ~Segment(){}
    static Segment *create_segment(buffer &b);
    virtual void load(buffer &b) = 0;
};

template <class T>
struct SegmentObjects : public Segment
{
    vector<T> objects;

    virtual void load(buffer &b)
    {
        for (int i = 0; i < n_objects; i++)
        {
            T r;
            r.load(b);
            objects.push_back(r);
        }
    }
};

struct Vector4
{
    float x = 0;
    float y = 0;
    float z = 0;
    float w = 0;
};

struct Common
{
    Vector4     m_rotate_z[3];
    Vector4     position;
    
    void load(buffer &b)
    {
        READ(b, m_rotate_z);
        READ(b, position);
    }
};

struct MapObject : public Common
{
    char name1[0x20];
    char name2[0x20];

    void load(buffer &b)
    {
        Common::load(b);

        READ(b, name1);
        READ(b, name2);
    }
};

struct MapObjectWithArray : public MapObject
{
    uint32_t len = 0;
    vector<uint32_t> unk7;

    void load(buffer &b)
    {
        MapObject::load(b);

        READ(b, len);
        unk7.resize(len);
        for (int i = 0; i < len; i++)
            READ(b, unk7[i]);
    }
};

struct Sound : public Common
{
    uint32_t unk1[11];
    char name1[0x14];

    void load(buffer &b)
    {
        Common::load(b);
        
        READ(b, unk1);
        READ(b, name1);
    }
};

struct Road : public MapObjectWithArray {};
struct Tower : public MapObjectWithArray {};

#define KNOWN_OBJECT(name) \
    struct name : public MapObject {}

KNOWN_OBJECT(Surface);
KNOWN_OBJECT(Helper);
KNOWN_OBJECT(Shell);
KNOWN_OBJECT(Stone);
KNOWN_OBJECT(Explosion);
KNOWN_OBJECT(Image);
KNOWN_OBJECT(Music);
KNOWN_OBJECT(Anomaly);
KNOWN_OBJECT(Boundary);

#define UNKNOWN_OBJECT(name) \
    struct name : public MapObject { void load(buffer &b){ int pos = b.index(); assert(false); } }

UNKNOWN_OBJECT(Building);
UNKNOWN_OBJECT(Goods);
UNKNOWN_OBJECT(unk1);

struct Objects
{
    uint32_t n_segments = 0;
    vector<Segment *> segments;

    void load(buffer &b);
};
