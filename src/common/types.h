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

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using f32 = float;

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

    bool operator==(const vector3 &rhs) const
    {
        return std::tie(x,y,z) == std::tie(rhs.x,rhs.y,rhs.z);
    }
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
    uint32_t unk0; // racing
    uint32_t n_segs;
    std::string name;
    std::vector<weather> segments;

    void load(const buffer &b, bool aim_racing = false) {
        if (aim_racing) {
            READ(b, unk0);
        }
        READ(b, n_segs);
        segments.resize(n_segs);
        READ_STRING_N(b, name, 0xA0);
        for (auto &s : segments)
            s.load(b);
    }
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
    enum class TovType : uint32_t
    {
        RawMaterial,
        Consumables,
        SemiFinished,
    };

    std::string name;
    char condition_variable[0x40]; // when this var is set, we can access the good
    float unk1_2 = 0;
    float price = 0; // unk, quantity?
    float unk2[10];
    float buy_price; // initial
    float sell_price; // initial
    TovType type;
    bool use_in_production;
    bool unk3;
    bool unk4;
    bool unk5;

    void load(const buffer &b)
    {
        READ_STRING(b, name);
        if (gameType == GameType::Aim1)
            READ(b, condition_variable);
        else
            READ(b, unk1_2);
        READ(b, price);
        if (gameType == GameType::Aim1)
            READ(b, unk2);
        else
        {
            READ(b, buy_price);
            READ(b, sell_price);
            READ(b, type);
            READ(b, use_in_production);
            READ(b, unk3);
            READ(b, unk4);
            READ(b, unk5);
        }
    }
};

struct BuildingGoods
{
    std::string name;

    std::vector<Good> goods;

    void load(const buffer &b)
    {
        READ_STRING(b, name);
        b.read_vector(goods);
    }
};

struct MapMusic
{
    std::string mainTheme;
    std::string name2;

    std::vector<std::string> fightThemes;
    std::vector<std::string> insertionThemes;

    void load(const buffer &b)
    {
        READ_STRING(b, mainTheme);
        READ_STRING(b, name2);

        auto read_values = [&b](auto &v, auto &n)
        {
            for (uint32_t i = 0; i < n; i++)
                v.push_back(b.read_string());
        };

        uint32_t n1 = 0;
        READ(b, n1);
        read_values(fightThemes, n1);

        uint32_t n2 = 0;
        READ(b, n2);
        read_values(insertionThemes, n2);
    }
};

struct OrganizationConfig
{
    int count_in_group;
    std::vector<std::string> configs;

    void load(const buffer &b)
    {
        uint32_t n_configs = 0;
        READ(b, n_configs);
        configs.resize(n_configs, std::string(0x20, 0));
        for (uint32_t i = 0; i < n_configs; i++)
            READ_N(b, configs[i][0], 0x20);
    }
};

struct Organization
{
    std::string name;
    int count; // on map?
    float trade_war;
    float defence_attack;
    float average_rating;
    bool is_free;
    bool is_foreign;
    OrganizationConfig configs[3];

    uint32_t unk0 = 0;
    char unk1[0xE0 - 4-4-4-4*3-4-1-1];

    void load(const buffer &b);
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

#pragma pack(push, 1)
struct pak {
    static constexpr uint32_t default_block_size = 0x4000;

    struct segment {
        // some file offset? trash? crc? m1 has zlib crc table (png)?
        uint32_t unk1;
        uint32_t algorithm;
        uint32_t offset;
    };
    struct file_description {
        char name[0x50];
        uint32_t offset;
        uint32_t size;
    };

    uint32_t magic;
    uint16_t unk0;
    uint32_t n_files;
    uint32_t n_blocks;
    uint32_t block_size;
    uint32_t unk1;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct script {
    static constexpr uint32_t default_block_size = 16000;

    uint32_t file_size;
    uint32_t unk0{default_block_size}; // stack size? always 16000? // section bits?
    uint32_t raw_text_size;
    uint32_t nlines;
};
#pragma pack(pop)
