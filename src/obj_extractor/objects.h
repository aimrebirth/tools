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

#define FREAD(var) fread(&var, 1, sizeof(var), f)
#define FREAD_N(var, n) fread(&var, 1, n, f)

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
    uint32_t    segment_len;
    uint32_t    n_objects;

    virtual ~Segment(){}
    static Segment *create_segment(FILE *f);
    virtual void load(FILE *f) = 0;
};

template <class T>
struct SegmentObjects : public Segment
{
    vector<T> objects;

    virtual void load(FILE *f)
    {
        for (int i = 0; i < n_objects; i++)
        {
            T r;
            r.load(f);
            objects.push_back(r);
        }
    }
};

struct Vector4
{
    float x;
    float y;
    float z;
    float w;
};

struct Common
{
    Vector4     m_rotate_z[3];
    Vector4     position;
    
    void load(FILE *f)
    {
        FREAD(m_rotate_z);
        FREAD(position);
    }
};

struct MapObject : public Common
{
    char name1[0x20];
    char name2[0x20];

    void load(FILE *f)
    {
        Common::load(f);

        FREAD(name1);
        FREAD(name2);
    }
};

struct MapObjectWithArray : public MapObject
{
    uint32_t len;
    vector<uint32_t> unk7;

    void load(FILE *f)
    {
        MapObject::load(f);

        FREAD(len);
        unk7.resize(len);
        for (int i = 0; i < len; i++)
            FREAD(unk7[i]);
    }
};

struct Sound : public Common
{
    uint32_t unk1[11];
    char name1[0x14];

    void load(FILE *f)
    {
        Common::load(f);
        
        FREAD(unk1);
        FREAD(name1);
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
    struct name : public MapObject { void load(FILE *f){ int pos = ftell(f); assert(false); } }

UNKNOWN_OBJECT(Building);
UNKNOWN_OBJECT(Goods);
UNKNOWN_OBJECT(unk1);

struct Objects
{
    uint32_t n_segments;
    vector<Segment *> segments;

    void load(FILE *f);
};
