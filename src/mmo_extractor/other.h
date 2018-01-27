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

#include <variant>

struct MechGroup
{
    std::string name;
    std::string org;
    std::vector<std::string> mechanoids;

    // used only in m1.loc1 and for sinigr
    // probably an old field
    std::string org_ru;

    struct unk_type01
    {
        uint32_t unk0 = 0;
        float unk1 = 0;
    };

    std::variant<unk_type01, std::vector<uint32_t>, uint32_t> type_data;

    bool hidden;

    void load(const buffer &b)
    {
        READ_STRING(b, name);
        READ_STRING(b, org);
        uint32_t type = 0;
        READ(b, type);
        uint32_t number_of_mechanoids = 0;
        READ(b, number_of_mechanoids);
        READ_STRING_N(b, org_ru, 0x70);

        switch (type)
        {
        case 0:
        case 1:
        {
            unk_type01 t;
            READ(b, t.unk0);
            READ(b, t.unk1);
            type_data = t;
        }
            break;
        case 2:
        {
            std::vector<uint32_t> t;
            uint32_t len = 0;
            READ(b, len);
            t.resize(len);
            for (int i = 0; i < len; i++)
                READ(b, t[i]);
            type_data = t;
        }
            break;
        case 3: // 3 = free mechanoids only?
        case 4:
        {
            uint32_t t;
            READ(b, t);
            type_data = t;
        }
            break;
        default:
            assert(false);
        }

        for (int i = 0; i < number_of_mechanoids; i++)
        {
            std::string t;
            READ_STRING_N(b, t, 0x20);
            mechanoids.push_back(t);
        }
        READ(b, hidden);
    }
};

struct MechGroups
{
    std::vector<MechGroup> mechGroups;
    char unk0[0x30]; // prefix?

    void load(const buffer &b)
    {
        if (gameType == GameType::Aim2)
        {
            uint32_t length = 0;
            READ(b, length);
        }
        uint32_t n = 0;
        READ(b, n);
        READ(b, unk0);

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
