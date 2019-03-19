/*
 * AIM save_loader
 * Copyright (C) 2016 lzwdgc
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

#include <buffer.h>
#include <mat.h>
#include <objects.h>
#include <types.h>

#include <iostream>

// possible save operations

struct changes
{
    buffer out;

    std::string mech_org;
    float money = 0;
    bool upgrade_equ_for_player = false;

    void rewrite_mech_org(const buffer &b, std::string &org);
    void rewrite_money(const buffer &b);
    void rewrite_upgrade_equ_for_player(const buffer &b, uint32_t value);

};

extern changes save_changes;

// common structs

struct int_variable
{
    std::string name;
    uint32_t value;

    void load(const buffer &b);
};

template <class T = uint32_t>
struct single_int
{
    T val;

    void load(const buffer &b)
    {
        READ(b, val);
    }
};

struct single_string
{
    std::string name;

    void load(const buffer &b);
};

struct string_variable
{
    std::string name;
    std::string value;

    void load(const buffer &b);
};

// segments

struct segment
{
    virtual void load(const buffer &) = 0;
};

struct header_segment : public segment
{
    uint32_t unk1[3];
    std::string save_name;
    vector3f position;
    std::string mmp_file;
    std::string location_name;
    Common camera;
    uint32_t unk2[7];

    void load(const buffer &b);
};

struct string_segment : public segment
{
    std::string s;

    void load(const buffer &b);
};

struct screen_segment : public segment
{
    mat<color> screenshot{ 128, 128 };

    void load(const buffer &b);
};

struct scripts_segment : public segment
{
    struct script_entry
    {
        std::string name;
        std::string path;

        void load(const buffer &b);
    };

    struct script_entry_ext : public script_entry
    {
        uint32_t unk0[10];

        void load(const buffer &b);
    };

    std::vector<script_entry> bases;
    std::vector<script_entry> events;
    std::vector<script_entry_ext> unk0;
    std::vector<int_variable> int_variables;
    std::vector<string_variable> str_variables;

    void load(const buffer &b);
};

// todo
struct radar_segment : public segment
{
    struct radar
    {
        char unk0[0x1C4];

        void load(const buffer &b);
    };

    struct mechanoid5
    {
        std::string name;
        float unk0[6];

        void load(const buffer &b);
    };

    std::vector<radar> radars;
    uint16_t unk0;
    std::vector<mechanoid5> mechanoid5s;
    std::vector<single_string> orgs;

    void load(const buffer &b);
};

struct gamedata_segment : public segment
{
    struct loc
    {
        float unk0;
        uint32_t unk1[2];
        std::string name;

        void load(const buffer &b)
        {
            READ(b, unk0);
            READ(b, unk1);
            READ_STRING(b, name);
        }
    };

    struct org
    {
        std::string name;
        uint32_t unk0; // flags?

        void load(const buffer &b)
        {
            READ_STRING(b, name);
            READ(b, unk0);
        }
    };

    struct unk0s
    {
        float unk0[2];
        uint32_t unk1;

        void load(const buffer &b)
        {
            READ(b, unk0);
            READ(b, unk1);
        }
    };

    struct unk1s
    {
        std::string name; // loc part?
        char unk0[0xE0];
        uint32_t unk1;
        uint32_t unk2;
        std::string location; // loc

        void load(const buffer &b)
        {
            READ_STRING(b, name);
            READ(b, unk0);
            READ(b, unk1);
            READ(b, unk2);
            READ_STRING(b, location);
        }
    };

    char unk00[0x5C];
    std::vector<loc> locs;
    std::vector<org> orgs;
    uint32_t unk0;
    std::vector<unk0s> unk1;
    std::string base_name;
    uint32_t unk2;
    std::string user_name; // mech name?
    std::vector<unk1s> unk3;
    std::string icon; // clan icon?
    uint32_t unk4;

    void load(const buffer &b);
};

struct questlog_segment : public segment
{
    struct record
    {
        std::string name;
        std::string name_journal;
        uint32_t unk0[2];
        std::string endtime;

        void load(const buffer &b);
    };

    uint32_t unk0;

    std::vector<int_variable> int_variables;
    std::vector<record> events;
    std::vector<string_variable> story_quests;

    void load(const buffer &b);
};

struct trade_segment : public segment
{
    struct Price
    {
        std::string tov_name;
        uint32_t unk0;
        float mass;
        float price;
        uint32_t notrade;
        uint32_t type;
        float unk1;

        void load(const buffer &b);
    };

    struct BuildingPrice
    {
        std::string name;
        std::vector<Price> prices;
        uint32_t unk0;

        void load(const buffer &b);
    };

    uint32_t unk0 = 0;
    std::vector<BuildingPrice> buildingPrices;

    void load(const buffer &b);
};

struct env_segment : public segment
{
    char unk0[0x151];
    weather w;

    void load(const buffer &b);
};

struct orgrel_segment : public segment
{
    struct org_rep
    {
        char unk0[0x8];
        bool unk1;

        void load(const buffer &b);
    };

    std::vector<org_rep> org_reps;

    void load(const buffer &b);
};

struct others_segment : public segment
{
    uint32_t unk0;
    uint32_t unk1;

    void load(const buffer &b);
};

struct mech_segment : public segment
{
    struct equipment
    {
        uint8_t id;
        std::string name;
        uint32_t unk0;
        uint32_t unk1;

        void load(const buffer &b);
    };

    struct moddable_equipment
    {
        std::string name;
        ModificatorMask mask;

        void load(const buffer &b);
    };

    struct moddable_equipment2
    {
        uint8_t id;
        std::string name;
        uint32_t unk0; // health?
        ModificatorMask mask;

        void load(const buffer &b);
    };

    struct hold_item
    {
        enum class Type : uint32_t
        {
            Good = 0,
            Mechanoid = 2,
        };

        Type type;
        std::string name;
        uint32_t count;
        uint32_t unk0;
        uint32_t unk1;

        void load(const buffer &b);
    };

    struct ammo
    {
        std::string name;

        void load(const buffer &b);
    };

    struct ammo_count
    {
        std::string name;
        uint32_t count;

        void load(const buffer &b);
    };

    enum class MechFlags : uint32_t
    {
        unk0    = 0x1,
        unk1    = 0x0100,
        Dead    = 0x010000,
        Dead2   = 0x01000000,
    };

    struct mech
    {
        uint8_t id;
        std::string name;
        std::string name2;
        std::string org;
        std::string building;

        MechFlags flags;
        uint8_t unk11;
        float unk12[3];
        uint32_t unk13[3];
        uint8_t unk14;
        uint32_t unk15;
        uint32_t unk15_1;
        uint32_t unk16 = 0;

        std::vector<equipment> equipments;

        float unk40;
        uint32_t unk4[7];

        // glider
        // g_unk = glider unknown

        // mask works only for weapons
        moddable_equipment glider;
        moddable_equipment weapon1;
        moddable_equipment weapon2;
        moddable_equipment reactor1;
        moddable_equipment reactor2;
        moddable_equipment engine1;
        moddable_equipment engine2;
        moddable_equipment energy_shield;
        moddable_equipment armor;

        uint32_t g_unk0;
        uint32_t g_unk1 = 0;
        uint32_t g_unk2;

        std::vector<ammo> ammos;
        std::vector<ammo> ammos1;
        std::vector<ammo> ammos2;

        uint32_t g_unk3 = 0;

        // mask works for all except uarmor
        moddable_equipment2 ureactor1;
        moddable_equipment2 ureactor2;
        moddable_equipment2 uengine1;
        moddable_equipment2 uengine2;
        moddable_equipment2 uenergy_shield;
        moddable_equipment2 uarmor;
        moddable_equipment g_unk4;
        ModificatorMask glider_mask;

        std::vector<ammo_count> ammos3;

        float money;

        std::vector<hold_item> items;

        uint32_t g_unk6[28][3] = { 0 };
        float g_unk7 = 0;
        float g_unk8 = 0;
        uint32_t g_unk9 = 0;
        uint8_t g_unk10 = 0;

        void load(const buffer &b);

        bool isPlayer() const;
    };

    std::vector<mech> mechs;

    void load(const buffer &b);
};

// todo
struct groups_segment : public segment
{
    struct mech
    {
        std::string name;
        uint32_t unk0;
        float unk1[4];
        uint32_t unk2[2];
        float unk3;

        void load(const buffer &b);
    };

    struct group
    {
        vector3f pos;
        std::string org;
        std::string base;

        std::vector<mech> mechs;

        void load(const buffer &b);
    };

    std::vector<group> groups;

    void load(const buffer &b);
};

struct orgdata_segment : public segment
{
    struct org_config
    {
        std::vector<single_string> names;

        void load(const buffer &b);
    };

    struct orgdata
    {
        uint32_t unk0;
        std::string org;
        float unk1[4];
        uint32_t unk2[2];
        float rep[50];
        std::vector<org_config> configs;

        void load(const buffer &b);
    };

    std::vector<orgdata> orgdatas;

    void load(const buffer &b);
};

struct builds_segment : public segment
{
    struct build
    {
        std::string bld;
        std::string org;
        float unk1;

        void load(const buffer &b);
    };

    std::vector<build> builds;

    void load(const buffer &b);
};

// todo
struct orgs_segment : public segment
{
    struct org
    {
        void load(const buffer &b);
    };

    std::vector<org> orgs;

    void load(const buffer &b);
};

struct tradeeqp_segment : public segment
{
    struct Good
    {
        std::string tov_name;
        uint32_t unk0;
        uint32_t mask;
        float price;
        int32_t count;
        float probability;

        void load(const buffer &b);
    };

    struct bld
    {
        std::string name;
        std::vector<Good> prices;

        void load(const buffer &b);
    };

    uint32_t unk0 = 0;
    std::vector<Good> prices;
    std::vector<bld> blds;

    void load(const buffer &b);
};

// todo
struct objects_segment : public segment
{
    struct base
    {
        std::string name;
        uint32_t unk0[26];
        uint16_t unk1;

        void load(const buffer &b);
    };

    std::vector<base> bases;

    void load(const buffer &b);
};

// what is mms?
struct mms_state_segment : public segment
{
    std::string name;

    void load(const buffer &b);
};

// todo
struct mms_c_config_segment : public segment
{
    struct object
    {
        void load(const buffer &b) {}
    };

    std::vector<object> objects;

    void load(const buffer &b);
};

struct mms_world_data_segment : public segment
{
    float unk0;

    void load(const buffer &b);
};

struct mainmech_segment : public segment
{
    uint32_t unk0[9];

    void load(const buffer &b);
};

struct segment_desc
{
    std::string name;
    segment *seg = nullptr;

    void load(const buffer &b);
};

struct save
{
    uint32_t magic;
    uint32_t version;
    std::vector<segment_desc> segments;

    void load(const buffer &b);
};
