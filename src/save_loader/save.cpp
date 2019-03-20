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

#include "save.h"

#include <primitives/exceptions.h>

changes save_changes;

void changes::rewrite_mech_org(const buffer &b, std::string &org)
{
    if (mech_org.empty())
        return;

    // setpos
    out.seek(b.index());
    out.skip(-0x20);
    out.write(mech_org);

    org = mech_org;
}

void changes::rewrite_money(const buffer &b)
{
    if (money == 0)
        return;
    out.seek(b.index());
    out.skip(-4);
    out.write(money);
}

void changes::rewrite_upgrade_equ_for_player(const buffer &b, uint32_t value)
{
    if (!upgrade_equ_for_player)
        return;
    out.seek(b.index());
    out.skip(-4);
    out.write(value);
}

void int_variable::load(const buffer &b)
{
    READ_STRING(b, name);
    READ(b, value);
}

void single_string::load(const buffer &b)
{
    READ_STRING(b, name);
}

void string_variable::load(const buffer &b)
{
    READ_STRING(b, name);
    READ_STRING(b, value);
}

void header_segment::load(const buffer &b)
{
    READ(b, unk1);
    READ_STRING_N(b, save_name, 0x60);
    READ(b, position);
    READ_STRING_N(b, mmp_file, 0x110);
    READ_STRING(b, location_name);
    READ(b, camera);
    READ(b, unk2);
}

void string_segment::load(const buffer &b)
{
    READ_PASCAL_STRING(b, s);
}

void screen_segment::load(const buffer &b)
{
    b._read((uint8_t *)screenshot.getData().data(), screenshot.getBytesLength());
    // rows should be swapped now: bottom is the first row etc.
}

void scripts_segment::script_entry::load(const buffer &b)
{
    READ_STRING(b, name);
    READ_STRING_N(b, path, 0x80);
}

void scripts_segment::script_entry_ext::load(const buffer &b)
{
    READ_STRING(b, name);
    READ(b, unk0);
    READ_STRING_N(b, path, 0x80);
}

void scripts_segment::load(const buffer &b)
{
    b.read_vector(bases);
    b.read_vector(events);
    b.read_vector(unk0);
    b.read_vector(int_variables);
    b.read_vector(str_variables);
}

void radar_segment::radar::load(const buffer &b)
{
    READ(b, unk0);
}

void radar_segment::mechanoid5::load(const buffer &b)
{
    READ_STRING(b, name);
    READ(b, unk0);
}

void radar_segment::load(const buffer &b)
{
    b.read_vector(radars);
    READ(b, unk0);
    b.read_vector(mechanoid5s);
    b.read_vector(orgs);
}

void gamedata_segment::load(const buffer &b)
{
    READ(b, unk00);
    b.read_vector(locs);
    b.read_vector(orgs);
    //READ(b, unk0);
    b.read_vector(unk1);
    READ_STRING(b, base_name);
    READ(b, unk2);
    READ_PASCAL_STRING(b, user_name);
    b.read_vector(unk3);
    READ_STRING(b, icon);
    READ(b, unk4);
}

void questlog_segment::record::load(const buffer &b)
{
    READ_STRING(b, name);
    READ_STRING(b, name_journal);
    READ(b, unk0);
    READ_STRING(b, endtime);
}

void questlog_segment::load(const buffer &b)
{
    READ(b, unk0);
    b.read_vector(int_variables);
    b.read_vector(events);
    b.read_vector(story_quests);
}

void trade_segment::Price::load(const buffer &b)
{
    READ_STRING(b, tov_name);
    READ(b, unk0);
    READ(b, mass);
    READ(b, price);
    READ(b, notrade);
    READ(b, type);
    READ(b, unk1);
}

void trade_segment::BuildingPrice::load(const buffer &b)
{
    READ_STRING(b, name);
    b.read_vector(prices);
    READ(b, unk0);
}

void trade_segment::load(const buffer &b)
{
    READ(b, unk0);
    b.read_vector(buildingPrices);
}

void env_segment::load(const buffer &b)
{
    READ(b, unk0);
    w.load(b);
}

void orgrel_segment::org_rep::load(const buffer &b)
{
    READ(b, unk0);
    READ(b, unk1);
}

void orgrel_segment::load(const buffer &b)
{
    b.read_vector(org_reps);
    uint32_t unk0[2];
    READ(b, unk0);
}

void others_segment::load(const buffer &b)
{
    READ(b, unk0);
    READ(b, unk1);
}

void mech_segment::equipment::load(const buffer &b)
{
    READ(b, id);
    READ_STRING(b, name);
    READ(b, unk0);
    READ(b, unk1);
}

void mech_segment::moddable_equipment::load(const buffer &b)
{
    READ_STRING(b, name);
    READ(b, mask);
}

void mech_segment::moddable_equipment2::load(const buffer &b)
{
    READ(b, id);
    READ_STRING(b, name);
    READ(b, unk0);
    READ(b, mask);
}

void mech_segment::hold_item::load(const buffer &b)
{
    READ(b, type);
    READ_STRING(b, name);
    READ(b, count);
    READ(b, unk0);
    READ(b, unk1);
}

void mech_segment::ammo::load(const buffer &b)
{
    READ_STRING(b, name);
}

void mech_segment::ammo_count::load(const buffer &b)
{
    READ_STRING(b, name);
    READ(b, count);
}

void mech_segment::mech::glider_desc::load(const buffer &b)
{
    READ(b, unk15_1); // size?

    b.read_vector(equipments);

    READ(b, unk40);
    READ(b, unk4);

    //
    glider.load(b);
    //if (isPlayer())
        //save_changes.rewrite_upgrade_equ_for_player(b, 0x1666);
    weapon1.load(b);
    //if (isPlayer())
        //save_changes.rewrite_upgrade_equ_for_player(b, 0x2666);
    weapon2.load(b);
    //if (isPlayer())
        //save_changes.rewrite_upgrade_equ_for_player(b, 0x2666);
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
    //if (isPlayer())
    //save_changes.rewrite_upgrade_equ_for_player(b, 0x3666);
    ureactor2.load(b);
    //if (isPlayer())
    //save_changes.rewrite_upgrade_equ_for_player(b, 0x3666);
    uengine1.load(b);
    //if (isPlayer())
    //save_changes.rewrite_upgrade_equ_for_player(b, 0x4666);
    uengine2.load(b);
    //if (isPlayer())
    //save_changes.rewrite_upgrade_equ_for_player(b, 0x4666);
    uenergy_shield.load(b);
    //if (isPlayer())
    //save_changes.rewrite_upgrade_equ_for_player(b, 0x5666);
    uarmor.load(b);

    g_unk4.load(b);

    READ(b, glider_mask);
    //if (isPlayer())
    //save_changes.rewrite_upgrade_equ_for_player(b, 0x1666);

    b.read_vector(ammos3);

    //
    READ(b, money);
    //if (name == "PLAYER")
    //save_changes.rewrite_money(b);

    b.read_vector(items);

    READ(b, g_unk6);
}

void mech_segment::mech::load(const buffer &b)
{
    READ(b, id); // 3 for mech?
    /*switch (id)
    {
    case 3:
    {*/
        READ_STRING(b, name);
        READ_STRING(b, name2);
        READ_STRING(b, org);
        //save_changes.rewrite_mech_org(b, org);
        READ_STRING(b, building);

        READ(b, flags);
        READ(b, unk11);
        READ(b, unk12);
        READ(b, unk13);
        READ(b, unk14);

        auto f = (uint32_t)flags;
        if (!(f == 0x01000101 || f == 0x00000001 || f == 0x00000101 || f == 0x01000001))
        {
            READ(b, unk16);
            return;
        }

        READ(b, unk15);

        if (unk14 == 0)
            return;
    /*}
        break;
    case 1:
        break;
    default:
        std::cerr << "unknown mechanoid type: " << (int)id << "\n";
        break;
    }*/

    gl.load(b);

    // TODO

    // if (g_unk0 == 1)
    // if ((uint32_t)g_unk2 == 15)
    // if ((uint32_t)g_unk3 == 0)
    // if (unk13[0] == 5)
    //if (g_unk6[26][0] != 0 && strcmp((const char *)b.getPtr(), "GROUPS") != 0)
    {
        float g_unk7 = 0;
        float g_unk8 = 0;
        uint32_t g_unk9 = 0;
        uint8_t g_unk10 = 0;

        READ(b, g_unk7);
        /*if (g_unk7 != 0)
        {
        b.skip(-4);
        return;
        }*/
        READ(b, g_unk8);
        READ(b, g_unk9);
        READ(b, g_unk10);

        if (g_unk10 > 1)
            std::cerr << "g_unk10 > 1" << "\n";

        if (g_unk10)
            gl.load(b);
    }
}

bool mech_segment::mech::isPlayer() const
{
    return name == "PLAYER";
}

void mech_segment::load(const buffer &b)
{
    b.read_vector(mechs);
}

void groups_segment::mech::load(const buffer &b)
{
    READ_STRING(b, name);
    READ(b, unk0);
    READ(b, unk1);
    READ(b, unk2);
    READ(b, unk3);
}

void groups_segment::group::load(const buffer &b)
{
    uint32_t unk0;
    uint16_t unk1;
    float unk2[3];

    READ(b, pos);
    READ_STRING(b, org);
    READ_STRING(b, base);

    READ(b, unk0);
    READ(b, unk1);
    READ(b, unk2);

    b.read_vector(mechs);
}

void groups_segment::load(const buffer &b)
{
    b.read_vector(groups);
}

void orgdata_segment::org_config::load(const buffer &b)
{
    b.read_vector(names);
}

void orgdata_segment::orgdata::load(const buffer &b)
{
    READ(b, unk0);
    READ_STRING(b, org);
    READ(b, unk1);
    READ(b, unk2);
    READ(b, rep);
    b.read_vector(configs, 3);
}

void orgdata_segment::load(const buffer &b)
{
    b.read_vector(orgdatas);
}

void builds_segment::build::load(const buffer &b)
{
    READ_STRING(b, bld);
    READ_STRING(b, org);
    READ(b, unk1);
}

void builds_segment::load(const buffer &b)
{
    b.read_vector(builds);
}

void orgs_segment::org::base::mech::load(const buffer &b)
{
    READ_STRING(b, name);
}

void orgs_segment::org::base::load(const buffer &b)
{
    READ_STRING(b, name);
    READ(b, unk2);
    READ(b, unk3);
    READ(b, unk4);
    b.read_vector(mechs);

    u32 n;
    u32 unk0;
    READ(b, n);
    while (n--)
        READ(b, unk0);

    READ_STRING(b, org);
    READ(b, unk5);
    READ(b, unk6);
}

void orgs_segment::org::load(const buffer &b)
{
    READ(b, unk0);
    base.load(b);
}

void orgs_segment::load(const buffer &b)
{
    b.read_vector(orgs);
}

void tradeeqp_segment::Good::load(const buffer &b)
{
    READ_STRING(b, tov_name);
    READ(b, unk0);
    READ(b, mask);
    READ(b, price);
    READ(b, count);
    READ(b, probability);
}

void tradeeqp_segment::bld::load(const buffer &b)
{
    READ_STRING(b, name);
    b.read_vector(prices);
}

void tradeeqp_segment::load(const buffer &b)
{
    READ(b, unk0);
    b.read_vector(prices);
    b.read_vector(blds);
}

void objects_segment::object::load(const buffer &b)
{
    READ_STRING(b, owner);
    READ(b, type);
    READ(b, unk1);
    switch (type)
    {
    case 0x16: // trgun
    {
        b.skip(0x4);
        return;
    }
    }
    READ(b, unk01);
    READ(b, coords);
    READ(b, len0); // flags?

    if (len0 == 0)
        return;

    switch (type)
    {
    case 0x10: // ammo (mines etc.)
    {
        std::string name;
        READ_STRING(b, name);
        std::string owner;
        READ_STRING(b, owner);
        b.skip(0x1E);
    }
    break;
    case 0x15: // mech, money, tov
    {
        std::string what;
        u8 unk0;
        u32 unk1[2];
        std::string s1;
        u32 unk2;
        std::string s2;
        f32 unk3;

        READ_STRING(b, what);
        if (what == "TT_MONEY")
            ;
        else if (what == "TT_MECHANOID")
            ;
        else if (what == "TT_RAW_FRAGMENT")
            ;
        else if (what == "TT_CONTAINER")
            ;
        else if (what == "TT_FUNGUS")
            ;
        else
            std::cerr << "unknown object: " << what << "\n";

        READ(b, unk0);
        READ(b, unk1);
        READ_STRING(b, s1);
        READ(b, unk2);
        READ_STRING(b, s2);
        READ(b, unk3);
    }
    break;
    case 0x17: // trgate
    {
        b.skip(0x4);
        return;
    }
    case 0x1a: // mech in base
    {
        float unk3;
        u8 unk4; // also flags? 0x47 for in buildings, 0x4f

        float unk5[2][3];
        u32 flags;
        float unk10[2];

        // flags != 1
        float unk100[3];
        u32 len1;
        float unk11[4];
        u32 unk111;

        // flags == 1
        std::string unk12;

        READ(b, unk3);
        READ(b, unk4);
        READ(b, unk5);
        READ(b, flags);
        READ(b, unk10);

        //if (flags == 1)
        //{
            READ_STRING(b, unk12);
        //}
        /*else
        {
            READ(b, unk100);
            READ(b, len1);
            READ(b, unk11);
            READ(b, unk111);
        }*/

        std::vector<std::string> mechs;
        std::vector<std::string> mechs2;
        uint32_t n;
        READ(b, n);
        mechs.resize(n);
        for (int i = 0; i < n; i++)
            READ_STRING(b, mechs[i]);

        //  in production?
        //if (flags == 1)
        {
            std::string s;
            uint32_t n;
            READ(b, n);
            mechs2.resize(n);
            for (int i = 0; i < n; i++)
                READ_STRING(b, mechs2[i]);
        }
    }
    break;
    default:
        throw SW_RUNTIME_ERROR("unknown object in OBJECTS with type " + std::to_string(type));
    }
}

void objects_segment::load(const buffer &b)
{
    b.read_vector(objects);
}

void mms_state_segment::load(const buffer &b)
{
    READ_STRING(b, name);
}

void mms_c_config_segment::load(const buffer &b)
{
    while (!b.eof())
    {
        mech_segment::mech::glider_desc o;
        o.load(b);
        objects.push_back(o);
    }
}

void mms_world_data_segment::load(const buffer &b)
{
    READ(b, unk0);
}

void mainmech_segment::load(const buffer &b)
{
    READ(b, unk0);
}

void segment_desc::load(const buffer &b)
{
    READ_STRING(b, name);
    uint32_t magic;
    READ(b, magic);
    if (magic != (uint32_t)-2)
        throw SW_RUNTIME_ERROR("bad magic for segment: " + name);

    uint32_t len; // length of segment + 4 (sizeof(int) or internal segment length size)
    READ(b, len);
    uint32_t offset;
    READ(b, offset);
    uint32_t unk1;
    READ(b, unk1);
    uint32_t len2; // length of segment
    READ(b, len2);
    if (len != len2 + sizeof(len2))
        throw SW_RUNTIME_ERROR("bad length for segment: " + name);

    uint32_t start = b.index();
    uint32_t end = start + len2;

#define SWITCH(s, t)                                                                                                   \
    if (name == s)                                                                                                     \
    seg = new t
#define CASE(s, t) else SWITCH(s, t)
#define DEFAULT else

    SWITCH("HEADER", header_segment);
    CASE("CAPTION", string_segment);
    CASE("PLAYER", string_segment);
    CASE("SCREEN", screen_segment);
    CASE("SCRIPTS", scripts_segment);
    // CASE("RADAR", radar_segment);
    CASE("GAMEDATA", gamedata_segment);
    CASE("QUESTLOG", questlog_segment);
    CASE("TRADE", trade_segment);
    CASE("ENV", env_segment);
    CASE("ORGREL", orgrel_segment);
    CASE("OTHERS", others_segment);
    //CASE("MECH", mech_segment);
    // CASE("GROUPS", groups_segment);
    CASE("ORGDATA", orgdata_segment);
    CASE("BUILDS", builds_segment);
    CASE("ORGS", orgs_segment);
    CASE("TRADEEQP", tradeeqp_segment);
    CASE("OBJECTS", objects_segment);
    CASE("MMS_STATE", mms_state_segment);
    CASE("MMS_C_CONFIG", mms_c_config_segment); // (my) clan config
    CASE("MMS_WORLD_DATA", mms_world_data_segment);
    CASE("MAINMECH", mainmech_segment);
    DEFAULT
    {
        std::cout << "skipped " << name << " size = " << len2 << "\n";
        b.skip(len2);
        return;
    }

    seg->load(buffer(b, len2));
    if (b.index() != end)
        throw SW_RUNTIME_ERROR("bad segment read: " + name);

#undef SWITCH
#undef CASE
#undef DEFAULT
}

void save::load(const buffer &b)
{
    READ(b, magic);
    READ(b, version);

    while (!b.eof())
    {
        segment_desc sd;
        sd.load(b);
        if (sd.seg)
            segments.push_back(sd);
    }
}
