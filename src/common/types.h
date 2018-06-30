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
    Aim2,
    AimR,
};

inline GameType gameType = GameType::Aim2;

template <typename T>
struct vector3
{
    T x;
    T y;
    T z;
};

using vector3f = vector3<float>;

struct vector4
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

/*
somewhere from old demos
    [MAP_DEMO]
    filename=Data\Map\map1.bmp
    path=Data\Map\map1.pnt
    color=Data\Map\map1_color.bmp
    ;skyUp=00FFFF
    ;skyMiddle=404040
    ;skyDown=0
    ;cloud=FF8F8F
    skyUp=FF0080FF
    skyMiddle=FF808080
    skyDown=FF808080
    cloud=FF8F8F
*/
struct weather
{
    struct atmospheric_effects
    {
        vector3f wind;
        WeatherType weatherType;
        float strength;
        float duration;
        float probability;
    };

    std::string name;
    std::string unk0;
    uint32_t unk1[2];
    color smoke_1; //3?
    color smoke_3; //1?
    SmokeType smokeType;
    uint32_t unk2[3];
    std::string cloud_layer1;
    std::string cloud_layer2;
    float cloud_layer1_speed;
    float cloud_layer2_speed;
    vector3f cloud_layer1_direction;
    vector3f cloud_layer2_direction;
    std::string sun;
    color general_color;
    color sun_color;
    color moon_color;
    std::string moon;
    float probability;
    std::string day_night_gradient_name;
    std::string dawn_dusk_gradient_name;
    color dawn_dusk_color;
    atmospheric_effects effects;
    color smoke_2;
    color smoke_4;
    uint32_t slider_3;
    uint32_t slider_1;
    float unk8[11];

    void load(const buffer &b);
};

struct weather_group
{
    uint32_t n_segs;
    std::string name;
    std::vector<weather> segments;

    void load(const buffer &b);
};

struct water
{
    float unk0[6];
    std::string name1;
    uint32_t unk1;
    float unk2;
    uint32_t unk3[16];
    float unk4;
    std::string name2;
    uint32_t unk5[16];

    void load(const buffer &b);
};

struct water_group
{
    std::vector<water> segments;

    void load(const buffer &b);
};

struct Good
{
    std::string name;
    char unk1[0x40];
    float unk1_2 = 0;
    float price = 0;
    float unk2[10];
    float unk2_1[2];
    uint32_t unk2_2[2];

    void load(const buffer &b)
    {
        READ_STRING(b, name);
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
    std::string name;
    uint32_t n = 0;

    std::vector<Good> goods;

    void load(const buffer &b)
    {
        READ_STRING(b, name);
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
    std::string name1;
    std::string name2;

    std::vector<std::string> names1;
    std::vector<std::string> names2;

    void load(const buffer &b)
    {
        READ_STRING(b, name1);
        READ_STRING(b, name2);

        auto read_values = [&b](auto &v, auto &n)
        {
            for (int i = 0; i < n; i++)
                v.push_back(b.read_string());
        };

        uint32_t n1 = 0;
        READ(b, n1);
        read_values(names1, n1);

        uint32_t n2 = 0;
        READ(b, n2);
        read_values(names2, n2);
    }
};

struct OrganizationConfig
{
    uint32_t n_configs = 0;
    std::vector<std::string> configs;

    void load(const buffer &b)
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
    std::string name;
    char unk1[0xE0];
    OrganizationConfig configs[3];

    void load(const buffer &b)
    {
        READ(b, unk0);
        READ_STRING(b, name);
        READ(b, unk1);
        for (auto &c : configs)
            c.load(b);
    }
};

struct Organizations
{
    std::vector<Organization> organizations;

    void load(const buffer &b)
    {
        uint32_t len = 0;
        READ(b, len);
        uint32_t n = 0;
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
    std::string base_name;
    std::string org_name;
    uint32_t unk0 = 0;

    void load(const buffer &b)
    {
        READ_STRING(b, base_name);
        READ_STRING(b, org_name);
        READ(b, unk0);
    }
};

struct OrganizationBases
{
    std::vector<OrganizationBase> organizationBases;

    void load(const buffer &b)
    {
        uint32_t n = 0;
        READ(b, n);
        for (int i = 0; i < n; i++)
        {
            OrganizationBase s;
            s.load(b);
            organizationBases.push_back(s);
        }
    }
};

struct ModificatorMask
{
    enum class ItemType : uint8_t
    {
        Glider          = 1,
        Weapon          = 2,
        Reactor         = 3,
        Engine          = 4,
        EnergyShield    = 5,
    };

    uint8_t fight : 4;
    uint8_t trade : 4;
    uint8_t courier : 4;
    ItemType type : 4;

    uint16_t : 16;
};
