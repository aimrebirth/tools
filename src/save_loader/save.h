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

// possible save operations

struct changes
{
    buffer out;

    std::string mech_org;
    float money = 0;
    bool upgrade_equ_for_player = false;

    void rewrite_mech_org(const buffer &b, std::string &org)
    {
        if (mech_org.empty())
            return;

        // setpos
        out.seek(b.index());
        out.skip(-0x20);
        out.write(mech_org);

        org = mech_org;
    }

    void rewrite_money(const buffer &b)
    {
        if (money == 0)
            return;
        out.seek(b.index());
        out.skip(-4);
        out.write(money);
    }

    void rewrite_upgrade_equ_for_player(const buffer &b, uint32_t value)
    {
        if (!upgrade_equ_for_player)
            return;
        out.seek(b.index());
        out.skip(-4);
        out.write(value);
    }

} save_changes;

// common structs

struct int_variable
{
    std::string name;
    uint32_t value;

    void load(const buffer &b)
    {
        READ_STRING(b, name);
        READ(b, value);
    }
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

    void load(const buffer &b)
    {
        READ_STRING(b, name);
    }
};

struct string_variable
{
    std::string name;
    std::string value;

    void load(const buffer &b)
    {
        READ_STRING(b, name);
        READ_STRING(b, value);
    }
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

    void load(const buffer &b)
    {
        READ(b, unk1);
        READ_STRING_N(b, save_name, 0x60);
        READ(b, position);
        READ_STRING_N(b, mmp_file, 0x110);
        READ_STRING(b, location_name);
        READ(b, camera);
        READ(b, unk2);
    }
};

struct string_segment : public segment
{
    std::string s;

    void load(const buffer &b)
    {
        READ_PASCAL_STRING(b, s);
    }
};

struct screen_segment : public segment
{
    mat<color> screenshot{ 128, 128 };

    void load(const buffer &b)
    {
        b._read((uint8_t*)screenshot.getData().data(), screenshot.getBytesLength());
        // rows should be swapped now: bottom is the first row etc.
    }
};

struct scripts_segment : public segment
{
    struct script_entry
    {
        std::string name;
        std::string path;

        void load(const buffer &b)
        {
            READ_STRING(b, name);
            READ_STRING_N(b, path, 0x80);
        }
    };

    struct script_entry_ext : public script_entry
    {
        uint32_t unk0[10];

        void load(const buffer &b)
        {
            READ_STRING(b, name);
            READ(b, unk0);
            READ_STRING_N(b, path, 0x80);
        }
    };

    std::vector<script_entry> bases;
    std::vector<script_entry> events;
    std::vector<script_entry_ext> unk0;
    std::vector<int_variable> int_variables;
    std::vector<string_variable> str_variables;

    void load(const buffer &b)
    {
        b.read_vector(bases);
        b.read_vector(events);
        b.read_vector(unk0);
        b.read_vector(int_variables);
        b.read_vector(str_variables);
    }
};

// todo
struct radar_segment : public segment
{
    struct radar
    {
        char unk0[0x1C4];

        void load(const buffer &b)
        {
            READ(b, unk0);
        }
    };

    struct mechanoid5
    {
        std::string name;
        float unk0[6];

        void load(const buffer &b)
        {
            READ_STRING(b, name);
            READ(b, unk0);
        }
    };

    std::vector<radar> radars;
    uint16_t unk0;
    std::vector<mechanoid5> mechanoid5s;
    std::vector<single_string> orgs;

    void load(const buffer &b)
    {
        b.read_vector(radars);
        READ(b, unk0);
        b.read_vector(mechanoid5s);
        b.read_vector(orgs);
    }
};

// todo
struct gamedata_segment : public segment
{
    void load(const buffer &b)
    {
    }
};

struct questlog_segment : public segment
{
    struct record
    {
        std::string name;
        std::string name_journal;
        uint32_t unk0[2];
        std::string endtime;

        void load(const buffer &b)
        {
            READ_STRING(b, name);
            READ_STRING(b, name_journal);
            READ(b, unk0);
            READ_STRING(b, endtime);
        }
    };

    uint32_t unk0;

    std::vector<int_variable> int_variables;
    std::vector<record> events;
    std::vector<string_variable> story_quests;

    void load(const buffer &b)
    {
        READ(b, unk0);
        b.read_vector(int_variables);
        b.read_vector(events);
        b.read_vector(story_quests);
    }
};

struct trade_segment : public segment
{
    struct Price
    {
        std::string tov_name;
        float unk2[6];

        void load(const buffer &b)
        {
            READ_STRING(b, tov_name);
            READ(b, unk2);
        }
    };

    struct BuildingPrice
    {
        std::string name;
        std::vector<Price> prices;
        uint32_t unk0;

        void load(const buffer &b)
        {
            READ_STRING(b, name);
            b.read_vector(prices);
            READ(b, unk0);
        }
    };

    uint32_t unk0 = 0;
    std::vector<BuildingPrice> buildingPrices;

    void load(const buffer &b)
    {
        READ(b, unk0);
        b.read_vector(buildingPrices);
    }
};

// todo
struct env_segment : public segment
{
    void load(const buffer &b)
    {
    }
};

// todo
struct orgrel_segment : public segment
{
    struct org_rep
    {
        char unk1[0xE0];

        void load(const buffer &b)
        {
            READ(b, unk1);
        }
    };

    std::vector<org_rep> org_reps;

    void load(const buffer &b)
    {
        b.read_vector(org_reps);
    }
};

struct others_segment : public segment
{
    uint32_t unk0;
    uint32_t unk1;

    void load(const buffer &b)
    {
        READ(b, unk0);
        READ(b, unk1);
    }
};

struct mech_segment : public segment
{
    struct equipment
    {
        uint8_t id;
        std::string name;
        uint32_t unk0;
        uint32_t unk1;

        void load(const buffer &b)
        {
            READ(b, id);
            READ_STRING(b, name);
            READ(b, unk0);
            READ(b, unk1);
        }
    };

    struct moddable_equipment
    {
        std::string name;
        ModificatorMask mask;

        void load(const buffer &b)
        {
            READ_STRING(b, name);
            READ(b, mask);
        }
    };

    struct moddable_equipment2
    {
        uint8_t id;
        std::string name;
        uint32_t unk0; // health?
        ModificatorMask mask;

        void load(const buffer &b)
        {
            READ(b, id);
            READ_STRING(b, name);
            READ(b, unk0);
            READ(b, mask);
        }
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

        void load(const buffer &b)
        {
            READ(b, type);
            READ_STRING(b, name);
            READ(b, count);
            READ(b, unk0);
            READ(b, unk1);
        }
    };

    struct ammo
    {
        std::string name;

        void load(const buffer &b)
        {
            READ_STRING(b, name);
        }
    };

    struct ammo_count
    {
        std::string name;
        uint32_t count;

        void load(const buffer &b)
        {
            READ_STRING(b, name);
            READ(b, count);
        }
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

        void load(const buffer &b)
        {
            READ(b, id);
            READ_STRING(b, name);
            READ_STRING(b, name2);
            READ_STRING(b, org);
            save_changes.rewrite_mech_org(b, org);
            READ_STRING(b, building);

            READ(b, flags);
            READ(b, unk11);
            READ(b, unk12);
            READ(b, unk13);
            READ(b, unk14);

            auto f = (uint32_t)flags;
            if (!(f == 0x01000101 ||
                f == 0x00000001 ||
                f == 0x00000101 ||
                f == 0x01000001))
            {
                READ(b, unk16);
                return;
            }

            READ(b, unk15);

            if (unk14 == 0)
                return;

            READ(b, unk15_1); // size?

            b.read_vector(equipments);

            READ(b, unk40);
            READ(b, unk4);

            //
            glider.load(b);
            if (isPlayer())
                save_changes.rewrite_upgrade_equ_for_player(b, 0x1666);
            weapon1.load(b);
            if (isPlayer())
                save_changes.rewrite_upgrade_equ_for_player(b, 0x2666);
            weapon2.load(b);
            if (isPlayer())
                save_changes.rewrite_upgrade_equ_for_player(b, 0x2666);
            reactor1.load(b);
            reactor2.load(b);
            engine1.load(b);
            engine2.load(b);
            energy_shield.load(b);
            armor.load(b);

            // ?
            READ(b, g_unk0);
            if (g_unk0 == 0)
                READ(b, g_unk1);
            READ(b, g_unk2);

            if (g_unk0 == 0)
                b.read_vector(ammos, g_unk1);

            b.read_vector(ammos1, g_unk0);

            if (g_unk0 != 0 || g_unk1 != 0)
            {
                READ(b, g_unk3);
                if (g_unk0 > 0)
                {
                    uint32_t n;
                    b.skip(-8);
                    READ(b, n);
                    b.skip(4);
                    b.read_vector(ammos2, n);
                    std::vector<single_int<>> v;
                    b.read_vector(v, n);
                }
                if (g_unk1 > 0)
                {
                    g_unk1--;
                    while (g_unk1--)
                        READ(b, g_unk3);
                }
            }

            ureactor1.load(b);
            if (isPlayer())
                save_changes.rewrite_upgrade_equ_for_player(b, 0x3666);
            ureactor2.load(b);
            if (isPlayer())
                save_changes.rewrite_upgrade_equ_for_player(b, 0x3666);
            uengine1.load(b);
            if (isPlayer())
                save_changes.rewrite_upgrade_equ_for_player(b, 0x4666);
            uengine2.load(b);
            if (isPlayer())
                save_changes.rewrite_upgrade_equ_for_player(b, 0x4666);
            uenergy_shield.load(b);
            if (isPlayer())
                save_changes.rewrite_upgrade_equ_for_player(b, 0x5666);
            uarmor.load(b);

            g_unk4.load(b);

            READ(b, glider_mask);
            if (isPlayer())
                save_changes.rewrite_upgrade_equ_for_player(b, 0x1666);

            b.read_vector(ammos3);

            READ(b, money);
            if (name == "PLAYER")
                save_changes.rewrite_money(b);

            b.read_vector(items);

            READ(b, g_unk6);

            // TODO

            //if (g_unk0 == 1)
            //if ((uint32_t)g_unk2 == 15)
            //if ((uint32_t)g_unk3 == 0)
            //if (unk13[0] == 5)
            if (g_unk6[26][0] != 0 &&
                strcmp((const char *)b.getPtr(), "GROUPS") != 0)
            {

                READ(b, g_unk7);
                /*if (g_unk7 != 0)
                {
                    b.skip(-4);
                    return;
                }*/
                READ(b, g_unk8);
                READ(b, g_unk9);
                READ(b, g_unk10);
            }
        }

        bool isPlayer() const
        {
            return name == "PLAYER";
        }
    };

    std::vector<mech> mechs;

    void load(const buffer &b)
    {
        b.read_vector(mechs);
    }
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

        void load(const buffer &b)
        {
            READ_STRING(b, name);
            READ(b, unk0);
            READ(b, unk1);
            READ(b, unk2);
            READ(b, unk3);
        }
    };

    struct group
    {
        vector3f pos;
        std::string org;
        std::string base;
        float unk0[4];
        uint16_t unk1;

        std::vector<mech> mechs;

        void load(const buffer &b)
        {
            READ(b, pos);
            READ_STRING(b, org);
            READ_STRING(b, base);
            READ(b, unk0);
            READ(b, unk1);
            b.read_vector(mechs);
        }
    };

    std::vector<group> groups;

    void load(const buffer &b)
    {
        b.read_vector(groups);
    }
};

struct orgdata_segment : public segment
{
    struct org_config
    {
        std::vector<single_string> names;

        void load(const buffer &b)
        {
            b.read_vector(names);
        }
    };

    struct orgdata
    {
        uint32_t unk0;
        std::string org;
        float unk1[4];
        uint32_t unk2[2];
        float rep[50];
        std::vector<org_config> configs;

        void load(const buffer &b)
        {
            READ(b, unk0);
            READ_STRING(b, org);
            READ(b, unk1);
            READ(b, unk2);
            READ(b, rep);
            b.read_vector(configs, 3);
        }
    };

    std::vector<orgdata> orgdatas;

    void load(const buffer &b)
    {
        b.read_vector(orgdatas);
    }
};

struct builds_segment : public segment
{
    struct build
    {
        std::string bld;
        std::string org;
        float unk1;

        void load(const buffer &b)
        {
            READ_STRING(b, bld);
            READ_STRING(b, org);
            READ(b, unk1);
        }
    };

    std::vector<build> builds;

    void load(const buffer &b)
    {
        b.read_vector(builds);
    }
};

// todo
struct orgs_segment : public segment
{
    struct org
    {
        void load(const buffer &b)
        {
        }
    };

    std::vector<org> orgs;

    void load(const buffer &b)
    {
        b.read_vector(orgs);
    }
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

        void load(const buffer &b)
        {
            READ_STRING(b, tov_name);
            READ(b, unk0);
            READ(b, mask);
            READ(b, price);
            READ(b, count);
            READ(b, probability);
        }
    };

    struct bld
    {
        std::string name;
        std::vector<Good> prices;

        void load(const buffer &b)
        {
            READ_STRING(b, name);
            b.read_vector(prices);
        }
    };

    uint32_t unk0 = 0;
    std::vector<Good> prices;
    std::vector<bld> blds;

    void load(const buffer &b)
    {
        READ(b, unk0);
        b.read_vector(prices);
        b.read_vector(blds);
    }
};

// todo
struct objects_segment : public segment
{
    struct base
    {
        std::string name;
        uint32_t unk0[26];
        uint16_t unk1;

        void load(const buffer &b)
        {
            READ_STRING(b, name);
            READ(b, unk0);
            READ(b, unk1);
        }
    };

    std::vector<base> bases;

    void load(const buffer &b)
    {
        b.read_vector(bases);
    }
};

struct mms_state_segment : public segment
{
    std::string name;

    void load(const buffer &b)
    {
        READ_STRING(b, name);
    }
};

// todo
struct mms_c_config_segment : public segment
{
    struct object
    {
        void load(const buffer &b)
        {
        }
    };

    std::vector<object> objects;

    void load(const buffer &b)
    {
        // todo: insert config read from mech
        b.read_vector(objects);
    }
};

struct mms_world_data_segment : public segment
{
    float unk0;

    void load(const buffer &b)
    {
        READ(b, unk0);
    }
};

struct mainmech_segment : public segment
{
    uint32_t unk0[9];

    void load(const buffer &b)
    {
        READ(b, unk0);
    }
};

struct segment_desc
{
    std::string name;
    segment *seg = nullptr;

    void load(const buffer &b)
    {
        READ_STRING(b, name);
        uint32_t unk0;
        READ(b, unk0);
        uint32_t len;
        READ(b, len);
        uint32_t offset;
        READ(b, offset);
        uint32_t unk1;
        READ(b, unk1);
        uint32_t len2;
        READ(b, len2);

        uint32_t start = b.index();
        uint32_t end = start + len2;

#define SWITCH(s, t) if (name == s) seg = new t
#define CASE(s, t) else SWITCH(s, t)
#define DEFAULT else

        SWITCH("HEADER", header_segment);
        CASE("CAPTION", string_segment);
        CASE("PLAYER", string_segment);
        CASE("SCREEN", screen_segment);
        CASE("SCRIPTS", scripts_segment);
        //CASE("RADAR", radar_segment);
        //CASE("GAMEDATA", gamedata_segment);
        CASE("QUESTLOG", questlog_segment);
        CASE("TRADE", trade_segment);
        //CASE("ENV", env_segment);
        //CASE("ORGREL", orgrel_segment);
        CASE("OTHERS", others_segment);
        CASE("MECH", mech_segment);
        //CASE("GROUPS", groups_segment);
        CASE("ORGDATA", orgdata_segment);
        CASE("BUILDS", builds_segment);
        //CASE("ORGS", orgs_segment);
        CASE("TRADEEQP", tradeeqp_segment);
        //CASE("OBJECTS", objects_segment);
        CASE("MMS_STATE", mms_state_segment);
        //CASE("MMS_C_CONFIG", mms_c_config_segment); // (my) clan config
        CASE("MMS_WORLD_DATA", mms_world_data_segment);
        CASE("MAINMECH", mainmech_segment);
        DEFAULT
        {
            std::cout << "skipped " << name << " size = " << len2 << "\n";
            b.skip(len2);
            return;
        }

        seg->load(b);

#undef SWITCH
#undef CASE
#undef DEFAULT
    }
};

struct save
{
    uint32_t magick;
    uint32_t version;
    std::vector<segment_desc> segments;

    void load(const buffer &b)
    {
        READ(b, magick);
        READ(b, version);

        while (!b.eof())
        {
            segment_desc sd;
            sd.load(b);
            if (sd.seg)
                segments.push_back(sd);
        }
    }
};
