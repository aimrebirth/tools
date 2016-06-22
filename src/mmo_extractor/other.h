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

#include <buffer.h>
#include <types.h>

struct MechGroup
{
    std::string name;
    std::string org;
    uint32_t type1 = 0;
    uint32_t len1 = 0;
    char name1[0x70];
    //{3,4
        uint32_t unk30 = 0;
    //}
    //{2
        uint32_t len = 0;
        std::vector<uint32_t> unk11;
    //}
    //{1,0
        uint32_t unk20 = 0;
        uint32_t unk21 = 0;
    //}
    std::vector<std::string> configs;
    char unk100;

    void load(const buffer &b)
    {
        READ_STRING(b, name);
        READ_STRING(b, org);
        READ(b, type1);
        READ(b, len1);
        READ(b, name1);
        if (type1 == 3 || type1 == 4)
        {
            READ(b, unk30);
        }
        else if (type1 == 2)
        {
            READ(b, len);
            unk11.resize(len);
            for (int i = 0; i < len; i++)
                READ(b, unk11[i]);
        }
        else if (type1 == 1 || type1 == 0)
        {
            READ(b, unk20);
            READ(b, unk21);
        }
        else
            assert(false);
        configs.resize(len1, std::string(0x20, 0));
        for (int i = 0; i < len1; i++)
            READ_N(b, configs[i][0], 0x20);
        READ(b, unk100);
    }
};

struct MechGroups
{
    char prefix[0x30];
    std::vector<MechGroup> mechGroups;

    void load(const buffer &b)
    {
        if (gameType == GameType::Aim2)
        {
            uint32_t length = 0;
            READ(b, length);
        }
        uint32_t n = 0;
        READ(b, n);
        READ(b, prefix);

        for (int s = 0; s < n; s++)
        {
            MechGroup mg;
            mg.load(b);
            mechGroups.push_back(mg);
        }
    }
};

struct MapGoods
{
    uint32_t unk2 = 0;
    uint32_t unk3 = 0;

    std::vector<BuildingGoods> bgs;

    void load(const buffer &b)
    {
        uint32_t length = 0;
        READ(b, length);
        READ(b, unk2);
        if (gameType != GameType::Aim2)
            READ(b, unk3);

        uint32_t n = 0;
        READ(b, n);
        for (int i = 0; i < n; i++)
        {
            BuildingGoods bg;
            bg.load(b);
            bgs.push_back(bg);
            if (gameType == GameType::Aim2)
                READ(b, unk2);
        }
    }
};

struct MapSound
{
    std::string name;
    float unk1[4];
    uint32_t unk2 = 0;
    float unk3[4];

    void load(const buffer &b)
    {
        READ_STRING(b, name);
        READ(b, unk1);
        READ(b, unk2);
        READ(b, unk3);
    }
};

struct MapSounds
{
    std::vector<MapSound> sounds;

    void load(const buffer &b)
    {
        b.read_vector(sounds);
    }
};

struct Price
{
    enum class ItemType : uint32_t
    {
        Glider = 1,
        Equipment,
        Weapon,
        Ammo,
    };

    std::string tov_name;
    ItemType type;
    ModificatorMask mask;
    float price;
    float unk2 = 0.0f; // count ?
    float probability; // of appearence

    void load(const buffer &b)
    {
        READ_STRING(b, tov_name);
        READ(b, type);
        READ(b, mask);
        READ(b, price);
        READ(b, unk2);
        READ(b, probability);
    }
};

struct BuildingPrice
{
    std::string name;
    std::vector<Price> prices;

    void load(const buffer &b)
    {
        READ_STRING(b, name);
        b.read_vector(prices);
    }
};

struct BuildingPrices
{
    std::vector<Price> prices;
    std::vector<BuildingPrice> buildingPrices;

    void load(const buffer &b)
    {
        b.read_vector(prices);
        b.read_vector(buildingPrices);
    }
};

struct Prices
{
    uint32_t unk0 = 0;
    BuildingPrices buildingPrices;

    void load(const buffer &b)
    {
        uint32_t len = 0;
        READ(b, len);
        READ(b, unk0);
        buildingPrices.load(b);
    }
};
