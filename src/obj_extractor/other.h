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

using namespace std;

struct MechGroup
{
    char unk1[0x20];
    char unk2[0x20];
    uint32_t type1;
    uint32_t len1;
    char name1[0x70];
    //{3,4
        uint32_t unk30;
    //}
    //{2
        uint32_t len;
        vector<uint32_t> unk11;
    //}
    //{1,0
        uint32_t unk20;
        uint32_t unk21;
    //}
    vector<string> configs;
    char unk100;

    void load(FILE *f)
    {
        FREAD(unk1);
        FREAD(unk2);
        FREAD(type1);
        FREAD(len1);
        FREAD(name1);
        if (type1 == 3 || type1 == 4)
        {
            FREAD(unk30);
        }
        else if (type1 == 2)
        {
            FREAD(len);
            unk11.resize(len);
            for (int i = 0; i < len; i++)
                FREAD(unk11[i]);
        }
        else if (type1 == 1 || type1 == 0)
        {
            FREAD(unk20);
            FREAD(unk21);
        }
        else
            assert(false);
        configs.resize(len1, string(0x20, 0));
        for (int i = 0; i < len1; i++)
            FREAD_N(configs[i][0], 0x20);
        FREAD(unk100);
    }
};

struct MechGroups
{
    uint32_t n;
    char prefix[0x30];

    vector<MechGroup> mgs;

    void load(FILE *f)
    {
        FREAD(n);
        FREAD(prefix);

        for (int s = 0; s < n; s++)
        {
            MechGroup mg;
            mg.load(f);
            mgs.push_back(mg);
        }
    }
};

struct Good
{
    char name[0x20];
    char unk1[0x40];
    float price;
    float unk2[10];

    void load(FILE *f)
    {
        FREAD(name);
        FREAD(unk1);
        FREAD(price);
        FREAD(unk2);
    }
};

struct BuildingGoods
{
    char name[0x20];
    uint32_t n;

    vector<Good> goods;

    void load(FILE *f)
    {
        FREAD(name);
        FREAD(n);

        for (int i = 0; i < n; i++)
        {
            Good g;
            g.load(f);
            goods.push_back(g);
        }
    }
};

struct MapGoods
{
    uint32_t unk1;
    uint32_t unk2;
    uint32_t unk3;
    uint32_t n;

    vector<BuildingGoods> bgs;

    void load(FILE *f)
    {
        FREAD(unk1);
        FREAD(unk2);
        FREAD(unk3);
        FREAD(n);

        for (int i = 0; i < n; i++)
        {
            BuildingGoods bg;
            bg.load(f);
            bgs.push_back(bg);
        }
    }
};

struct MapMusic
{
    uint32_t unk1;
    char name1[0x20];
    char name2[0x20];

    uint32_t n1;
    vector<string> names1;

    uint32_t n2;
    vector<string> names2;

    void load(FILE *f)
    {
        FREAD(unk1);
        FREAD(name1);
        FREAD(name2);

        FREAD(n1);
        for (int i = 0; i < n1; i++)
        {
            char name[0x20];
            FREAD(name);
            names1.push_back(name);
        }

        FREAD(n2);
        for (int i = 0; i < n2; i++)
        {
            char name[0x20];
            FREAD(name);
            names2.push_back(name);
        }
    }
};

struct MapSound
{
    char name[0x20];
    float unk1[4];
    uint32_t unk2;
    float unk3[4];

    void load(FILE *f)
    {
        FREAD(name);
        FREAD(unk1);
        FREAD(unk2);
        FREAD(unk3);
    }
};

struct MapSounds
{
    uint32_t n;
    vector<MapSound> sounds;

    void load(FILE *f)
    {
        FREAD(n);
        for (int i = 0; i < n; i++)
        {
            MapSound s;
            s.load(f);
            sounds.push_back(s);
        }
    }
};