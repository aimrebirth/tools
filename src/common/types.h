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

#pragma once

#include <stdint.h>

#include "buffer.h"
#include "color.h"

enum class GameType
{
    Aim1,
    Aim2
};
extern GameType gameType;

struct vector3
{
    float x = 0;
    float y = 0;
    float z = 0;
};

struct Vector4
{
    float x = 0;
    float y = 0;
    float z = 0;
    float w = 0;
};

enum class WeatherType : uint32_t
{
    rain = 0x1,
    snow = 0x2,
    storm = 0x4,
};

enum class SmokeType : uint32_t
{
    none,
    exp,
    biexp,
    linear,
};

struct weather
{
    struct atmospheric_effects
    {
        vector3 wind;
        WeatherType weatherType;
        float strength;
        float duration;
        float probability;
    };

    char name[0x20];
    char unk0[0x20];
    uint32_t unk1[2];
    color smoke_1; //3?
    color smoke_3; //1?
    SmokeType smokeType;
    uint32_t unk2[3];
    char cloud_layer1[0x20];
    char cloud_layer2[0x20];
    float cloud_layer1_speed;
    float cloud_layer2_speed;
    vector3 cloud_layer1_direction;
    vector3 cloud_layer2_direction;
    char sun[0x20];
    color general_color;
    color sun_color;
    color moon_color;
    char moon[0x20];
    float probability;
    char day_night_gradient_name[0x20];
    char dawn_dusk_gradient_name[0x20];
    color dawn_dusk_color;
    atmospheric_effects effects;
    color smoke_2;
    color smoke_4;
    uint32_t slider_3;
    uint32_t slider_1;
    float unk8[11];

    void load(buffer &b);
};

struct weather_group
{
    uint32_t n_segs;
    char name[0xA0];
    std::vector<weather> segments;

    void load(buffer &b);
};

struct water
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

struct water_group
{
    std::vector<water> segments;

    void load(buffer &b);
};

struct Good
{
    char name[0x20];
    char unk1[0x40];
    float unk1_2 = 0;
    float price = 0;
    float unk2[10];
    float unk2_1[2];
    uint32_t unk2_2[2];

    void load(buffer &b)
    {
        READ(b, name);
        if (gameType == GameType::Aim1)
            READ(b, unk1);
        else
            READ(b, unk1_2);
        READ(b, price);
        if (gameType == GameType::Aim1)
            READ(b, unk2);
        else
        {
            READ(b, unk2_1);
            READ(b, unk2_2);
        }
    }
};

struct BuildingGoods
{
    char name[0x20];
    uint32_t n = 0;

    std::vector<Good> goods;

    void load(buffer &b)
    {
        READ(b, name);
        READ(b, n);

        for (int i = 0; i < n; i++)
        {
            Good g;
            g.load(b);
            goods.push_back(g);
        }
    }
};

struct MapMusic
{
    char name1[0x20];
    char name2[0x20];

    uint32_t n1 = 0;
    std::vector<std::string> names1;

    uint32_t n2 = 0;
    std::vector<std::string> names2;

    void load(buffer &b)
    {
        READ(b, name1);
        READ(b, name2);

        READ(b, n1);
        for (int i = 0; i < n1; i++)
        {
            char name[0x20];
            READ(b, name);
            names1.push_back(name);
        }

        READ(b, n2);
        for (int i = 0; i < n2; i++)
        {
            char name[0x20];
            READ(b, name);
            names2.push_back(name);
        }
    }
};

struct OrganizationConfig
{
    uint32_t n_configs = 0;
    std::vector<std::string> configs;

    void load(buffer &b)
    {
        READ(b, n_configs);
        configs.resize(n_configs, std::string(0x20, 0));
        for (int i = 0; i < n_configs; i++)
            READ_N(b, configs[i][0], 0x20);
    }
};

struct Organization
{
    uint32_t unk0 = 0;
    char name[0x20];
    char unk1[0xE0];
    OrganizationConfig configs[3];

    void load(buffer &b)
    {
        READ(b, unk0);
        READ(b, name);
        READ(b, unk1);
        for (auto &c : configs)
            c.load(b);
    }
};

struct Organizations
{
    uint32_t len = 0;
    uint32_t n = 0;
    std::vector<Organization> organizations;

    void load(buffer &b)
    {
        READ(b, len);
        READ(b, n);
        for (int i = 0; i < n; i++)
        {
            Organization s;
            s.load(b);
            organizations.push_back(s);
        }
    }
};

struct OrganizationBase
{
    char base_name[0x20];
    char org_name[0x20];
    uint32_t unk0 = 0;

    void load(buffer &b)
    {
        READ(b, base_name);
        READ(b, org_name);
        READ(b, unk0);
    }
};

struct OrganizationBases
{
    uint32_t n = 0;
    std::vector<OrganizationBase> organizationBases;

    void load(buffer &b)
    {
        READ(b, n);
        for (int i = 0; i < n; i++)
        {
            OrganizationBase s;
            s.load(b);
            organizationBases.push_back(s);
        }
    }
};
